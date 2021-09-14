#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <iostream>
#include "global.cpp"

void send_request(int sockfd, Global::Commands command, char *filename = NULL) {
  char request_buffer[REQUEST_SIZE];
  //make request
  request_buffer[0] = command;
  if(filename != NULL){
    strcpy(request_buffer+1, filename);
  }
  send(sockfd, request_buffer, REQUEST_SIZE, 0); // TODO : doesn't work necessarily
}
// Download file from server
void download_file(int sockfd, char *filename) {
  send_request(sockfd, Global::DOWNLOAD, filename);
  FILE *fp;
  fp = fopen(filename, "w");
  
  if(fp==NULL){
      std::cout<<"Can't create file"<<std::endl;
  }
  write_file(sockfd, fp);   
}

void list_files(int sockfd) {
  send_request(sockfd, Global::LIST);

  //receive file list
  char filename[FILENAME_SIZE];
  while (1) {
    int bytes_received = recv(sockfd, filename, FILENAME_SIZE, 0);
    if (bytes_received <= 0){ 
      break;
    }
    std::cout << filename;
    bzero(filename, FILENAME_SIZE);
  }
} 

void delete_file(int sockfd, char *filename) {
  send_request(sockfd, Global::DELETE, filename);
}

int main(int argc, char** argv) {
  char *ip = "127.0.0.1";
  int port = 8000;
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

  // download test
  // download_file(sockfd,"meetfinal.mp4");

  //list test
  // list_files(sockfd);

  //delete test
  delete_file(sockfd, "ab.txt");
}