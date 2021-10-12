#include <iostream>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <filesystem>
#include <pthread.h>
#include <semaphore.h>
#include <set>

#include "filesystem.h"
#include "global.cpp"

#define QUEUE_LIM 10

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
std::set<std::string> lockedFiles;

class ClientSocket : public Socket {
    public:
    ClientSocket(int serverSocketDescriptor) {
        socklen_t addressSize = sizeof(address);
        fileDescriptor = accept(serverSocketDescriptor, (struct sockaddr *)&address, &addressSize);
        if(fileDescriptor < 0) {
            std::cout << "[!] Failure in Connection Reception" << std::endl;
            return;
        }
    }

    void uploadFile(char *fileName) {
        std::string filePath = STORAGE_PATH + fileName;

        pthread_mutex_lock(&lock);
        lockedFiles.insert(fileName);
        pthread_mutex_unlock(&lock);

        FILE *fp = fopen(filePath.c_str(), "w");
        readFile(fp);
        fclose(fp);

        pthread_mutex_lock(&lock);
        lockedFiles.erase(fileName);
        pthread_mutex_unlock(&lock); 
    }

    void downloadFile(char *fileName) {
        while(true){
            pthread_mutex_lock(&lock);
            if(lockedFiles.count(fileName) == 0) {
                pthread_mutex_unlock(&lock);
                break;    
            }
            pthread_mutex_unlock(&lock);
            sleep(SLEEP_TIME); 
        }
        std::string filePath = STORAGE_PATH + fileName;
        FILE* fp = fopen(filePath.c_str(), "r"); 
        if(fp == NULL) {
            std::cout << "File not found" << std::endl;
            close( getDescriptor());
            return; 
        }
        sendFile(fp); 
        fclose(fp);
    }

    void listFiles() {
        std::string fileList = "";
        for (const auto &entry : std::filesystem::directory_iterator(STORAGE_PATH)) {
            fileList += (entry.path().string().substr(STORAGE_PATH.size()) + "\n");
        }
        sendLong(fileList.size()+1);
        sendData((void *)fileList.c_str(), fileList.size()+1);
        close( getDescriptor());
    }

    void deleteFile(char *fileName) {
        std::string filePath = STORAGE_PATH + fileName;
        FILE* fp = fopen(filePath.c_str(), "r"); 
        if(fp == NULL) {
            std::cout << "File not found" << std::endl;
            return; 
        }
        std::string shellCommand = "rm ";
        shellCommand += filePath; 
        system(shellCommand.c_str());
    }

    void renameFile(char *fileName) {
        std::string filePath = STORAGE_PATH + fileName;
        FILE* fp = fopen(filePath.c_str(), "r"); 
        if(fp == NULL) {
            std::cout<< "File not found" <<std::endl;
            return; 
        }
        
        long len;
        readLong(&len);
        char *requestBuffer = (char *)malloc(len * sizeof(char)); 
        readData(requestBuffer, len); 
        char *newFileName = requestBuffer; 

        std::string shellCommand = "mv ";
        shellCommand += filePath;
        shellCommand += " ";
        shellCommand += STORAGE_PATH + newFileName; 
        system(shellCommand.c_str());
    }
};

class ServerSocket : public Socket {
    public:
    ServerSocket() {
        fileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
        if(fileDescriptor < 0) {
            std::cout <<"[-]Error in socket"<<std::endl;
            exit(1);
        }
        std::cout << "[+]Server socket created successfully." <<std::endl;
    }

    void setAddress(int _PORT, const char* _IP) {        
        address.sin_family = AF_INET;
        address.sin_port = _PORT;
        address.sin_addr.s_addr = inet_addr(_IP);
    }

    void bindSocket() {
        int err = bind(fileDescriptor, (struct sockaddr*)&address, sizeof(address));
        if(err < 0) {
            std::cout << "[-]Error in bind" << std::endl;
            exit(1);
        }
        std::cout << "[+]Binding successful." << std::endl;
    }

    void startListening() {
        if(listen(fileDescriptor, QUEUE_LIM) == 0) {
            std::cout << "[+]Listening...." << std::endl;
            startServing();
        } 
        else {
            std::cout << "[-]Error in listening" << std::endl;
            exit(1);
        }
    }

    void startServing() {
        while(true) {
            ClientSocket *newClient = new ClientSocket(fileDescriptor);
            pthread_t newThread;
            if(pthread_create(&newThread, NULL, handleRequest, (void *)newClient) != 0 ) {
                std::cout << "[!] Failure in thread creation!!" << std::endl;
            }
        }
    }

    static void* handleRequest(void *arg) {
        ClientSocket* sock = (ClientSocket *)(arg);
        long len;
        sock->readLong(&len);
        char *requestBuffer = (char *)malloc(len * sizeof(char));
        sock->readData(requestBuffer, len);

        char *fileName = requestBuffer + 1;

        switch(requestBuffer[0]) {
            case Global::UPLOAD: 
                sock->uploadFile(fileName); 
                break; 
            case Global::DOWNLOAD: 
                sock->downloadFile(fileName);
                break; 
            case Global::RENAME: 
                sock->renameFile(fileName); 
                break; 
            case Global::LIST: 
                sock->listFiles(); 
                break; 
            case Global::DELETE: 
                sock->deleteFile(fileName); 
                break; 
            default: 
                perror("unknown command\n"); 
        }
        pthread_exit(NULL);
    }   
};

int main(int argc, char **argv){
    if(argc != 3) {
        std::cout << "Usage: ./server <IP> <PORT>" << std::endl;
        return 0;
    }

    ServerSocket serverSocket;

    serverSocket.setAddress(atoi(argv[2]), argv[1]);

    serverSocket.bindSocket();

    serverSocket.startListening();
    return 0;
}