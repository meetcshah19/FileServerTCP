#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <iostream>

#include "oopsglobal.cpp"
#include "client.h"

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

    void establishConnection(int _PORT, std::string _IP) {
        address.sin_family = AF_INET;
        address.sin_port = _PORT;
        address.sin_addr.s_addr = inet_addr(IP);

        int err = connect(fileDescriptor, (struct sockaddr*)&address, sizeof(address));
        if(err == -1) {
            std::cout<<"[-]Error in socket"<<std::endl;
            exit(1);
        }
        std::cout<<"[+]Connected to Server."<<std::endl;
    }


    void sendRequest(Global::Commands command, char *fileName = NULL) {
        //make request
        sendLong(REQUEST_SIZE); 
        char  requestBuffer[REQUEST_SIZE];
        requestBuffer[0] = command;
        if(fileName != NULL){
            strcpy( requestBuffer+1, fileName);
        }
        sendData( requestBuffer, REQUEST_SIZE);
    }

    // Download file from server
    void downloadFile(char *fileName) {
        sendRequest(Global::DOWNLOAD, fileName);
        FILE *fp = fopen(fileName, "w");
        
        if(fp == NULL){
            std::cout<<"Can't create file"<<std::endl;
            return; 
        }

        readFile(fp);   
        fclose(fp);   
    }

    void uploadFile(char *filePath) {
        FILE *fp = fopen(filePath, "r"); 
        if(fp == NULL) {
            perror("File does not exist\n"); 
            return; 
        }
        char fileName[FILENAME_SIZE] = {0};
        int last_index = strlen(filePath) - 1;
        for(; last_index >= 0; last_index--) if(filePath[last_index] == '/') {
            break; 
        } 
        strcpy(fileName, filePath + last_index + 1); 

        sendRequest( Global::UPLOAD, fileName);
        sendFile( fp);
        fclose(fp); 
        std::cout << "[+]File data sent successfully." << std::endl;

    }

    void listFiles() {
        sendRequest( Global::LIST);
        //receive file list
        long len = 0; 
        readLong( &len); 
        char *filesList = (char *)malloc(len * sizeof(char)); 
        readData(filesList, len); 
        std::cout << filesList << std::endl; 
    } 

    void deleteFile(char *fileName) {
        sendRequest( Global::DELETE, fileName);
    }

    void renameFile(char *fileName) {
        sendRequest(Global::RENAME, fileName);
        char* new_file_name = "abracadabra.txt";  //TODO: remove hardcode
        sendLong(strlen(new_file_name));
        sendData((void *)new_file_name, strlen(new_file_name));
    }
};

class MenuHandler {
    public: 
    void printMenu() {
        std::cout << "[i] \t Main Menu \n\n" << std::endl;
        std::cout << "[1] \t Upload File: Press U / u "<< std::endl;
        std::cout << "[2] \t Download File: Press D / d " << std::endl;
        std::cout << "[3] \t List Files: Press L / l " << std::endl;
        std::cout << "[4] \t Delete/Remove File: Press R / r  " << std::endl;
        std::cout << "[5] \t Rename/Change Name of File: Press C / c " << std::endl;
        std::cout << "[6] \t Press Any other key to Exit " << std::endl;
    }

    void handleChoice(char choice, ServerSocket& serverSocket) {
        switch(choice) {
            case 'U':
            case 'u': 
                        serverSocket.uploadFile("test.txt");
                        break;
            case 'D':
            case 'd': 
                        serverSocket.downloadFile("abracadabra.txt");
                        break;
            case 'L':
            case 'l': 
                        serverSocket.listFiles();
                        break;
            case 'R':
            case 'r': 
                        serverSocket.deleteFile("test.txt");
                        break;
            case 'C':
            case 'c': 
                        serverSocket.renameFile("test.txt");
                        break;
            default:
                        std::cout << "Terminating the Program" << std::endl << std::endl;
                        exit(0);
        }
    }
};

int main(int argc, char** argv) {
    ServerSocket serverSocket;
    serverSocket.establishConnection(PORT, IP);

    MenuHandler M;
    while(true) {
        M.printMenu();
        char choice;
        std::cin >> choice;
        M.handleChoice(choice, serverSocket);
    }
}