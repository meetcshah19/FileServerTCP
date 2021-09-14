#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <iostream>
#include <filesystem>
#include <string> 
#include <unistd.h>

const int REQUEST_SIZE = 256; 
const int SIZE = 1024; 
const int FILENAME_SIZE = 256; 
const std::string STORAGE_PATH = "files/";

namespace Global {
    enum Commands {
        UPLOAD,
        DOWNLOAD,
        LIST,
        DELETE,
        RENAME,
    };
}

void write_file(int sockfd, FILE *fp){
  int n;
  char buffer[SIZE];
  
  while (1) {
    n = recv(sockfd, buffer, SIZE, 0);
    
    if (n <= 0){ 
      break;
    }
    fprintf(fp, "%s", buffer);
    bzero(buffer, SIZE);
  }

  fclose(fp); 
}

void send_file(int sockfd,FILE *fp){
    int n;
    char buffer[SIZE] = {0};  

    while(fgets(buffer, SIZE, fp) != NULL) {
        // std::cout << "in send file" << std::endl; 
        int bytes_sent = send(sockfd, buffer, sizeof(buffer), 0);
        if ( bytes_sent == -1) {
            perror("[-]Error in sending file.");
            exit(1);
        }
        bzero(buffer, SIZE);
    }
}

