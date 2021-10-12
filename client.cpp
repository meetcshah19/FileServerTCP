#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <iostream>

#include "global.cpp"

int PORT;
std::string IP;


class ServerSocket : public Socket {
    public:
    ServerSocket() {
        fileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
        if(fileDescriptor < 0) {
            std::cout <<"[-]Error in socket"<<std::endl;
            exit(1);
        }
    }

    void establishConnection(int PORT, std::string IP) {
        address.sin_family = AF_INET;
        address.sin_port = PORT;
        address.sin_addr.s_addr = inet_addr(IP.c_str());

        int err = connect(fileDescriptor, (struct sockaddr*)&address, sizeof(address));
        if(err == -1) {
            std::cout<<"[-]Error in socket"<<std::endl;
            exit(1);
        }
    }


    void sendRequest(Global::Commands command, const char *fileName = NULL) {
        //make request
        sendLong(REQUEST_SIZE); 
        char  requestBuffer[REQUEST_SIZE];
        requestBuffer[0] = command;
        if(fileName != NULL){
            strcpy( requestBuffer+1, fileName);
        }
        sendData(requestBuffer, REQUEST_SIZE);
    }

    // Download file from server
    void downloadFile(const char *fileName) {
        sendRequest(Global::DOWNLOAD, fileName);
        FILE *fp = fopen(fileName, "w");
        
        if(fp == NULL){
            std::cout<<"Can't create file"<<std::endl;
            return; 
        }

        readFile(fp);   
        fclose(fp);  
        std::cout << "[+] File Downloaded Successfully" << std::endl; 
    }

    void uploadFile(const char *filePath) {
        FILE *fp = fopen(filePath, "r"); 
        if(fp == NULL) {
            perror("File does not exist\n"); 
            return; 
        }
        char fileName[FILENAME_SIZE] = {};
        int last_index = strlen(filePath) - 1;
        for(; last_index >= 0; last_index--) if(filePath[last_index] == '/') {
            break; 
        } 
        strcpy(fileName, filePath + last_index + 1); 

        sendRequest( Global::UPLOAD, fileName);
        sendFile( fp);
        fclose(fp); 
        std::cout << "[+] File uploaded successfully." << std::endl;

    }

    void listFiles() {
        sendRequest( Global::LIST);
        //receive file list
        long len = 0; 
        readLong( &len); 
        char *filesList = (char *)malloc(len * sizeof(char)); 
        readData(filesList, len);
        std::cout << "----------" << std::endl; 
        std::cout << "FILE LIST " << std::endl;
        std::cout << "----------" << std::endl; 
        std::cout << filesList << std::endl; 
    } 

    void deleteFile(const char *fileName) {
        sendRequest( Global::DELETE, fileName);
        std::cout << "[+] File Deleted Successfully" << std::endl;
    }

    void renameFile(const char *fileName) {
        std::cout << "Enter new file name: ";
        std::string newFileName;
        std::cin >> newFileName;
        std::cerr << newFileName.c_str() << std::endl;
        sendRequest(Global::RENAME, fileName);
        sendLong(strlen(newFileName.c_str())+1);
        sendData((void *)newFileName.c_str(), strlen(newFileName.c_str())+1);
        std::cout << "[+] File Renamed Successfully" << std::endl;
    }
};

class MenuHandler {
    public: 
    void printMenu() {
        std::cout << "--------------------------------------" << std::endl;
        std::cout << "[i] \t Main Menu " << std::endl << std::endl;
        std::cout << "[1] \t Upload File: Press U / u "<< std::endl;
        std::cout << "[2] \t Download File: Press D / d " << std::endl;
        std::cout << "[3] \t List Files: Press L / l " << std::endl;
        std::cout << "[4] \t Delete/Remove File: Press R / r  " << std::endl;
        std::cout << "[5] \t Rename/Change Name of File: Press C / c " << std::endl;
        std::cout << "[6] \t Press Any other key to Exit " << std::endl;
        std::cout << "--------------------------------------" << std::endl;
    }

    std::string readFileName() {
        std::string fileName;
        std::cin >> fileName;
        return fileName;
    }

    void handleChoice(char choice) {
        std::string fileName;
        ServerSocket serverSocket;
        switch(choice) {
            case 'U':
            case 'u': 
                        serverSocket.establishConnection(PORT, IP);  
                        std::cout << "Enter file path: ";
                        fileName = readFileName();
                        serverSocket.uploadFile(fileName.c_str());
                        break;
            case 'D':
            case 'd': 
                        serverSocket.establishConnection(PORT, IP);  
                        std::cout << "Enter file name: ";
                        fileName = readFileName();
                        serverSocket.downloadFile(fileName.c_str());
                        break;
            case 'L':
            case 'l': 
                        serverSocket.establishConnection(PORT, IP);  
                        serverSocket.listFiles();
                        break;
            case 'R':
            case 'r': 
                        serverSocket.establishConnection(PORT, IP);  
                        std::cout << "Enter file name: ";
                        fileName = readFileName();
                        serverSocket.deleteFile(fileName.c_str());
                        break;
            case 'C':
            case 'c': 
                        serverSocket.establishConnection(PORT, IP);  
                        std::cout << "Enter file name: ";
                        fileName = readFileName();
                        serverSocket.renameFile(fileName.c_str());
                        break;
            default:
                        std::cout << "Terminating the Program" << std::endl << std::endl;
                        exit(0);
        }
    }
};

int main(int argc, char** argv) {
    if(argc != 3) {
        std::cout << "USAGE: ./client <SERVER_IP> <SERVER_PORT> " << std::endl;
        return 0;
    }
    
    IP = argv[1];
    PORT = atoi(argv[2]);

    MenuHandler M;
    while(true) {  
        M.printMenu();
        char choice;
        std::cin >> choice;
        M.handleChoice(choice);
    }
}