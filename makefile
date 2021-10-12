.PHONY: default
default: help

all: client server
	@echo "* Done"

client:
	@echo "* Building Client"
	@g++ -std=c++17 -pthread client.cpp -o client

server:
	@echo "* Building Server"
	@g++ -std=c++17 -pthread server.cpp -o server -lstdc++fs

clean:
	@rm  client server 1>/dev/null 2>&1
	@echo "* Done"

help:
	@echo "File-Server: A file-server built in C++"
	@echo ""
	@echo "* server: Build the server"
	@echo "* client: Build the client"
	@echo "* all:    Build the server and client"
	@echo "* clean:  Remove project binaries"
	@echo ""