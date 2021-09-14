#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <iostream>
#include "global.cpp"

void send_request(int sockfd, Global::Commands command, char *filename = NULL) {
  //make request
  send_long(sockfd, REQUEST_SIZE); 

  char request_buffer[REQUEST_SIZE];
  request_buffer[0] = command;
  if(filename != NULL){
    strcpy(request_buffer+1, filename);
  }

  send_data(sockfd, request_buffer, REQUEST_SIZE);
}

// Download file from server
void download_file(int sockfd, char *filename) {
  send_request(sockfd, Global::DOWNLOAD, filename);
  FILE *fp = fopen(filename, "w");
  
  if(fp == NULL){
      std::cout<<"Can't create file"<<std::endl;
      return; 
  }

  read_file(sockfd, fp);   
  fclose(fp);   
}

void list_files(int sockfd) {
  send_request(sockfd, Global::LIST);

  //receive file list
  long len = 0; 
  read_long(sockfd, &len); 
  char *files_list = (char *)malloc(len * sizeof(char)); 
  read_data(sockfd, files_list, len); 
  printf("%s", files_list); 
} 

int main(int argc, char** argv) {
  char *ip = "127.0.0.1";
  int port = 6969;
  int e;

  //connect to socket
  int sockfd;
  struct sockaddr_in server_addr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd < 0) {
    std::cout<<"[-]Error in socket"<<std::endl;
    exit(1);
  }
  std::cout<<"[+]Server socket created successfully."<<std::endl;

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = port;
  server_addr.sin_addr.s_addr = inet_addr(ip);

  e = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
  if(e == -1) {
    std::cout<<"[-]Error in socket"<<std::endl;
    exit(1);
  }
 std::cout<<"[+]Connected to Server."<<std::endl;

  //download test
  download_file(sockfd,"meetfinal.mp4");


}