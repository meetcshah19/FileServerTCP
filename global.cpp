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

bool send_data(int sock, void *buf, int buflen) {
    unsigned char *pbuf = (unsigned char *) buf;

    while (buflen > 0) {
        int bytes_sent = send(sock, pbuf, buflen, 0);
        if(bytes_sent < 0) {
          return false; 
        }

        pbuf += bytes_sent;
        buflen -= bytes_sent;
    }

    return true;
}

bool send_long(int sock, long value) {
    value = htonl(value);
    return send_data(sock, &value, sizeof(value));
}

bool send_file(int sock, FILE *f) {
    fseek(f, 0, SEEK_END);
    long filesize = ftell(f);
    rewind(f);
    if (filesize == EOF)
        return false;
    if (!send_long(sock, filesize))
        return false;
    if (filesize > 0) {
        char buffer[1024];
        do {
            size_t num = filesize > sizeof(buffer) ? sizeof(buffer): filesize;
            num = fread(buffer, 1, num, f);
            if (num < 1)
                return false;
            if (!send_data(sock, buffer, num))
                return false;
            filesize -= num;
        } while (filesize > 0);
    }
    return true;
}

bool read_data(int sock, void *buf, int buflen) {
    unsigned char *pbuf = (unsigned char *) buf;

    while (buflen > 0) {
        int num = recv(sock, pbuf, buflen, 0);
        if (num <= 0)
            return false;

        pbuf += num;
        buflen -= num;
    }

    return true;
}

bool read_long(int sock, long *value) {
    if (!read_data(sock, value, sizeof(value)))
        return false;
    *value = ntohl(*value);
    return true;
}

bool read_file(int sock, FILE *f) {
    long filesize;
    if (!read_long(sock, &filesize))
        return false;
    if (filesize > 0) {
        char buffer[1024];
        do {
            int num = filesize > sizeof(buffer) ? sizeof(buffer): filesize;
            if (!read_data(sock, buffer, num))
                return false;
            int offset = 0;
            do {
                size_t written = fwrite(&buffer[offset], 1, num-offset, f);
                if (written < 1)
                    return false;
                offset += written;
            } while (offset < num);

            filesize -= num;
        } while (filesize > 0);
    }

    return true;
}