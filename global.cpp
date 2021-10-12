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
const float SLEEP_TIME = 0.5;
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

#define print(x) std::cerr << #x << " is " << x << std::endl 

class Socket {
    protected:
    int fileDescriptor;
    struct sockaddr_in address;
    public:

    int getDescriptor() {
        return fileDescriptor;
    }
    bool sendData(void *buf, int buflen) {
        unsigned char *pbuf = (unsigned char *) buf;
        while (buflen > 0) {
            int bytes_sent = send(fileDescriptor, pbuf, buflen, 0);
            if(bytes_sent < 0) {
            return false; 
            }
            pbuf += bytes_sent;
            buflen -= bytes_sent;
        }
        return true;    
    }
    bool sendLong(long value) {
        value = htonl(value);
        return sendData(&value, sizeof(value));
    }
    bool sendFile(FILE *f) {
        fseek(f, 0, SEEK_END);
        long fileSize = ftell(f);
        rewind(f);
        if (fileSize == EOF)
            return false;
        if (!sendLong(fileSize))
            return false;
        if (fileSize > 0) {
            char buffer[1024];
            do {
                size_t num = fileSize > sizeof(buffer) ? sizeof(buffer): fileSize;
                num = fread(buffer, 1, num, f);
                if (num < 1)
                    return false;
                if (!sendData(buffer, num))
                    return false;
                fileSize -= num;
            } while (fileSize > 0);
        }
        return true;
    }
    bool readData(void *buf, int buflen) {
        unsigned char *pbuf = (unsigned char *) buf;
        while (buflen > 0) {
            int num = recv(fileDescriptor, pbuf, buflen, 0);
            if (num <= 0)
                return false;
            pbuf += num;
            buflen -= num;
        }
        return true;
    }
    bool readLong(long *value) {
        if (!readData(value, sizeof(value)))
            return false;
        *value = ntohl(*value);
        return true;       
    }
    bool readFile(FILE *f) {
        long fileSize;
        if (!readLong(&fileSize))
            return false;
        if (fileSize > 0) {
            char buffer[1024];
            do {
                int num = fileSize > sizeof(buffer) ? sizeof(buffer): fileSize;
                if (!readData(buffer, num))
                    return false;
                int offset = 0;
                do {
                    size_t written = fwrite(&buffer[offset], 1, num-offset, f);
                    if (written < 1)
                        return false;
                    offset += written;
                } while (offset < num);

                fileSize -= num;
            } while (fileSize > 0);
        }

        return true;    
    }    
};