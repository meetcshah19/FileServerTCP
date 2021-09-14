#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <filesystem>
#include <string> 
#include "global.cpp"

// Send file to client
void download_file(int sockfd, char *filename) {
    std::string file_path = STORAGE_PATH + filename;
    FILE* fp = fopen(file_path.c_str(), "r"); 
    if(fp == NULL) {
      std::cout<<"File not found"<<std::endl;
      close(sockfd);
      return; 
    }

    send_file(sockfd, fp); 
    fclose(fp); 
}

void upload_file(int sockfd, char *filename) {
  std::string file_path = STORAGE_PATH + filename;
  FILE *fp = fopen(file_path.c_str(), "w");

  read_file(sockfd, fp);
  fclose(fp);
}

void list_files(int sockfd) {
  std::string file_list = ""; 
	for (const auto &entry : std::filesystem::directory_iterator(STORAGE_PATH)) {
        file_list += (entry.path().string().substr(STORAGE_PATH.size()) + "\n");
  }
  send_long(sockfd, file_list.size()+1);
  send_data(sockfd, (void *)file_list.c_str(), file_list.size()+1);
  close(sockfd);
}

void delete_file(int sockfd, char *filename) {
  std::string file_path = STORAGE_PATH + filename;
  FILE* fp = fopen(file_path.c_str(), "r"); 
  if(fp == NULL) {
		std::cout<<"File not found"<<std::endl;
		return; 
	}
  std::string shell_command = "rm ";
  shell_command += file_path; 
  system(shell_command.c_str());
}

void rename_file(int sockfd, char *filename) {
  std::string file_path = STORAGE_PATH + filename;
  FILE* fp = fopen(file_path.c_str(), "r"); 
  if(fp == NULL) {
		std::cout<<"File not found"<<std::endl;
		return; 
	}
  long len;
  read_long(sockfd, &len);
  char *request_buffer = (char *)malloc(len * sizeof(char)); 
  read_data(sockfd, request_buffer, len); 
  char *new_filename = request_buffer; 

  std::string shell_command = "mv ";
  shell_command += file_path;
  shell_command += " ";
  shell_command += STORAGE_PATH + new_filename; 
  system(shell_command.c_str());
}






void handle_request(int sockfd) {
  long len; 
  read_long(sockfd, &len); 
  char *request_buffer = (char *)malloc(len * sizeof(char)); 
  read_data(sockfd, request_buffer, len); 
  char *filename = request_buffer + 1; 
  
  switch(request_buffer[0]) {
    case Global::UPLOAD: 
		upload_file(sockfd, filename); 
 		break; 
    case Global::DOWNLOAD: 
		download_file(sockfd, filename);
		break; 
    case Global::RENAME: 
		rename_file(sockfd, filename); 
		break; 
    case Global::LIST: 
		list_files(sockfd); 
		break; 
    case Global::DELETE: 
		delete_file(sockfd, filename); 
		break; 
    default: perror("unknown command\n"); 
  } 
}

int main(){
  char *ip = "127.0.0.1";
  int port = 6969;
  int e;

  int sockfd, new_sock;
  struct sockaddr_in server_addr, new_addr;
  socklen_t addr_size;

  // create server socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd < 0) {
    std::cout <<"[-]Error in socket"<<std::endl;
    exit(1);
  }
  std::cout <<"[+]Server socket created successfully."<<std::endl;

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = port;
  server_addr.sin_addr.s_addr = inet_addr(ip);

  e = bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
  if(e < 0) {
    std::cout <<"[-]Error in bind"<<std::endl;
    exit(1);
  }
  std::cout <<"[+]Binding successfull."<<std::endl;

  if(listen(sockfd, 10) == 0){
	std::cout <<"[+]Listening...."<<std::endl;
  } else {
	std::cout <<"[-]Error in listening"<<std::endl;
    exit(1);
  }

// handle connections
  while(true) {
    addr_size = sizeof(new_addr);
    new_sock = accept(sockfd, (struct sockaddr*)&new_addr, &addr_size);
    handle_request(new_sock);
  }

  return 0;
}