#include <iostream>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <filesystem>
#include <pthread.h>
#include <semaphore.h>

#include "server.h"
#include "thread_pool.h"
#include "filesystem.h"
#include "oopsglobal.cpp"

#define QUEUE_LIM 10


sem_t alp, bet;
pthread_t threads[MAX_WORKERS];
pthread_mutex_t lock;
int currentConnections = 0;
size_t numThreads = 0;

     
void initLock() {
    if (pthread_mutex_init(&lock, NULL) != 0) {
        std::cerr << "\n ERROR: Mutex initialization failed";
        exit(1);
    }
}

class ServerSocket;
class ClientSocket;

struct Arg {
    ServerSocket *S;
    ClientSocket *C;
    Arg(ServerSocket *A, ClientSocket *B) {
        S = A;
        C = B;
    }
};

void *globalHandleRequest(void *);


class ClientSocket : public Socket {
    public:
    ClientSocket(int serverSocketDescriptor) {
        socklen_t addressSize = sizeof(address);
        fileDescriptor = accept(serverSocketDescriptor, (struct sockaddr *)&address, &addressSize);
        if(fileDescriptor < 0) {
            std::cout << "[!] Failure in Connection Reception" << std::endl;
            return;
        }
        else {
            std::cout << "[+] Connection Established with Client" << std::endl;
        }
    }

    void uploadFile(char *fileName) {
        std::string filePath = STORAGE_PATH + fileName;
        FILE *fp = fopen(filePath.c_str(), "w");
        readFile(fp);
        fclose(fp);
    }

    void downloadFile(char *fileName) {
        std::string filePath = STORAGE_PATH + fileName;
        FILE* fp = fopen(filePath.c_str(), "r"); 
        if(fp == NULL) {
            std::cout<<"File not found"<<std::endl;
            close( getDescriptor());
            return; 
        }
        sendFile(fp); 
        fclose(fp);
    }

    void listFiles() {
        std::string file_list = "";
        for (const auto &entry : std::filesystem::directory_iterator(STORAGE_PATH)) {
            file_list += (entry.path().string().substr(STORAGE_PATH.size()) + "\n");
        }
        sendLong(file_list.size()+1);
        sendData((void *)file_list.c_str(), file_list.size()+1);
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
            std::cout<<"File not found"<<std::endl;
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
        initLock();
        while(true) {
            ClientSocket newClient(fileDescriptor);
            Arg A(this, &newClient);
            if(pthread_create(&threads[numThreads++], NULL, &globalHandleRequest, (void *)&A) != 0 ) {
                std::cout << "[!] Failure in thread creation: " << numThreads << std::endl;
            }
            if (numThreads >= 50) {
                numThreads = 0;
                while( numThreads < 50 ) {
                    pthread_join(threads[numThreads++], NULL);
                }
                numThreads = 0;
            }
        }
    }

    void handleRequest(ClientSocket *sock) {
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
    }   
};

void *globalHandleRequest(void *arg) {
    pthread_mutex_lock(&lock);
    Arg A = *((Arg *)(arg));
    A.S->handleRequest(A.C);
    pthread_mutex_unlock(&lock);
    pthread_exit(NULL);
}

int main() {
    ServerSocket serverSocket;

    serverSocket.setAddress(PORT, IP);

    serverSocket.bindSocket();

    serverSocket.startListening();
    return 0;
}