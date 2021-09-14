#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <iostream>
#include "global.cpp"

// Download file from server
void download_file(int sockfd, char *filename) {
  char request_buffer[REQUEST_SIZE];
  //make request
  request_buffer[0] = Global::DOWNLOAD;
  strcpy(request_buffer+1, filename);
  send(sockfd, request_buffer, REQUEST_SIZE, 0); // TODO : doesn't work necessarily
  FILE *fp;
  fp = fopen(filename, "w");
  
  if(fp==NULL){
      std::cout<<"Can't create file"<<std::endl;
  }
  write_file(sockfd, fp);   
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