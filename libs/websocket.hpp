/*
    websocket.hpp - source code of

    ================
    Websocket in C++
    ================

    Free to use, free to modify, free to redistribute.
    Created by : Jishan Ali Mondal

    This is a header-only library.
    created for only PIwebVNC
    * This code was created entirely for the most optimized performance for PIwebVNC *
    * May not be suitable for other projects *
    version 1.0.0
*/

#ifndef WEBSOCKET_HPP
#define WEBSOCKET_HPP

#include <iostream>
#include <thread>
#include <string.h>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "sha1.hpp"
#include "base64.hpp"
#include "httpPage.hpp"

#define PORT 8080

class Websocket
{
    static int client_socket[30];
    static int ws_client_socket[30];

public:
    static int clients;
    std::thread begin(int port = PORT);
    void sendFrame(char *img, long size, int sid = -1);
    void sendFrame(char *img, char *options, long size1, long size2, int sid = -1);
    void sendText(char *text, int sid = -1);
    void onConnect(void (*ptr)(int sid));
    void onMessage(void (*ptr)(void *data, int sid));
    int ready = 0;
    bool stop = false;

private:
    int socketPort = 8080;
    int server_fd;
    void connections();
    void handshake(unsigned char *d, int sd, int sid);
    void sendRaw(int startByte, char *data, long imgSize, int sid);
    void sendRaw(int startByte, char *data, char *data2, long data1Size, long data2Size, int sid);
    void decode(unsigned char *src,char * dest);
    void (*callBack)(int sid) = NULL;
    void (*callBackMsg)(void *data, int sid) = NULL;
};
int Websocket::client_socket[30] = {0};
int Websocket::ws_client_socket[30] = {0};
int Websocket::clients = 0;
void Websocket::onConnect(void (*ptr)(int sid))
{
    callBack = ptr;
}
void Websocket::onMessage(void (*ptr)(void *data, int sid))
{
    callBackMsg = ptr;
}
std::thread Websocket::begin(int port)
{
    parseHttpPage();
    this->socketPort = port;
    std::thread t(&Websocket::connections, this);
    return t;
}

void Websocket::connections()
{
    int new_socket, valread = 0;
    struct sockaddr_in address;
    int opt = 1, max_sd = 0;
    int addrlen = sizeof(address);
    unsigned char buffer[1024] = {0};
    fd_set readfds;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(this->socketPort);

    if ((this->server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(this->server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    if (bind(this->server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(this->server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    std::cout << "[I] Waiting for connections ...port " << this->socketPort << " opened (HTTP/WS)\n";
    while (!this->stop)
    {
        FD_ZERO(&readfds);
        FD_SET(this->server_fd, &readfds);
        max_sd = this->server_fd;
        for (int i = 0; i < 5; i++)
        {
            int sd = client_socket[i];
            if (sd > 0)
                FD_SET(sd, &readfds);
            if (sd > max_sd)
                max_sd = sd;
        }
        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR))
        {
            perror("bind failed");
        }
        if (FD_ISSET(this->server_fd, &readfds))
        {
            if ((new_socket = accept(this->server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            this->ready = 1;
            printf("[I] New connection , socket fd is %d , ip is : %s , port : %d \n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
            for (int i = 0; i < 30; i++)
            {
                if (client_socket[i] == 0)
                {
                    client_socket[i] = new_socket;
                    clients++;
                    break;
                }
            }
        }
        else
        {
            for (int i = 0; i < 30; i++)
            {
                if (client_socket[i] == 0)
                    continue;
                int sd = client_socket[i];
                if (FD_ISSET(sd, &readfds))
                {
                    if ((valread = read(sd, buffer, 1024)) == 0)
                    {
                        getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
                        printf("Host disconnected , ip %s , port %d \n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                        close(sd);
                        client_socket[i] = 0;
                        this->ws_client_socket[i] = 0;
                        clients--;
                    }
                    else
                    {
                        buffer[valread] = '\0';
                        if (buffer[0] == 'G' && buffer[1] == 'E' && buffer[2] == 'T')
                        {
                            handshake(buffer, sd, i);
                        }
                        else if ((int)buffer[0] == 136)
                        {
                            close(sd);
                            client_socket[i] = 0;
                            this->ws_client_socket[i] = 0;
                            clients--;
                            printf("Host disconnected : %d\n", sd);
                        }
                        else
                        {
                            if (callBackMsg != NULL)
                            {
                                char inputData[200]={0};
                                decode(buffer , inputData);
                                callBackMsg(inputData, i);
                            }
                        }
                    }
                }
            }
        }
    }
    std::cout << "[I] Server closed\n";
}

void Websocket::handshake(unsigned char *data, int sd, int sid)
{
    bool flag = false;
    char *ptr = strtok((char *)data, "\n");
    std::string key;
    while (ptr != NULL)
    {
        if (ptr[0] == 'S' && ptr[4] == 'W' && ptr[14] == 'K')
        {
            int i = 18, len = strlen(ptr) - 2;
            while (i++ < len)
            {
                key += ptr[i];
            }
            SHA1 sha;
            std::string result = sha.hash(key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
            result = base64_encode(sha.encode());
            // creating result set
            std::string response = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nServer: PIwebVNC (by Jishan)\r\nConnection: Upgrade\r\n";
            response += "Sec-WebSocket-Accept: " + result + "\r\n\r\n";
            char char_array[response.length() + 1];
            strcpy(char_array, response.c_str());
            send(sd, char_array, response.length(), 0);
            this->ws_client_socket[sid] = 1;
            this->ready = 1;
            flag = true;
            break;
        }
        ptr = strtok(NULL, "\n");
        // callback
    }
    if (!flag)
    {
        //send html file by invoking liteHTTP class [todo]
        send(client_socket[sid], htmlPage.index_html.c_str(),htmlPage.size, 0);
        close(sd);
        client_socket[sid] = 0;
        this->ws_client_socket[sid] = 0;
        clients--;
    }
    else{
        if (callBack != NULL)
        {
            (*callBack)(sid);
        }
    }
}

void Websocket::sendText(char *text, int sid)
{
    long textSize = strlen((char *)text);
    sendRaw(129, text, textSize, sid);
}

void Websocket::sendFrame(char *img, long size, int sid)
{
    sendRaw(130, img, size, sid);
}
void Websocket::sendFrame(char *img, char *options, long size1, long size2, int sid)
{
    sendRaw(130, img, options, size1, size2, sid);
}
void Websocket::sendRaw(int startByte, char *data, long imgSize, int sid)
{
    int moded = 0;
    char header[11];
    header[0] = startByte;
    if (imgSize <= 125)
    {
        header[1] = imgSize;
        moded = 2;
    }
    else if (imgSize >= 126 && imgSize <= 65535)
    {
        header[1] = 126;
        header[2] = ((imgSize >> 8) & 255);
        header[3] = (imgSize & 255);
        moded = 4;
    }
    else
    {
        header[1] = 127;
        header[2] = ((imgSize >> 56) & 255);
        header[3] = ((imgSize >> 48) & 255);
        header[4] = ((imgSize >> 40) & 255);
        header[5] = ((imgSize >> 32) & 255);
        header[6] = ((imgSize >> 24) & 255);
        header[7] = ((imgSize >> 16) & 255);
        header[8] = ((imgSize >> 8) & 255);
        header[9] = (imgSize & 255);
        moded = 10;
    }
    if (sid != -1)
    {
        send(client_socket[sid], header, moded, 0); // for websocket header
        send(client_socket[sid], data, imgSize, 0); // for websocket data
        return;
    }
    for (int i = 0; i < 30; i++)
    {
        if (client_socket[i] == 0 || this->ws_client_socket[i] == 0)
            continue;
        // max optimisation for sending data
        send(client_socket[i], header, moded, 0); // for websocket header
        send(client_socket[i], data, imgSize, 0); // for websocket data
    }
}

void Websocket::sendRaw(int startByte, char *data1, char *data2, long data1Size, long data2Size, int sid)
{
    int moded = 0;
    long imgSize = data1Size + data2Size;
    char header[11];
    header[0] = startByte;
    if (imgSize <= 125)
    {
        header[1] = imgSize;
        moded = 2;
    }
    else if (imgSize >= 126 && imgSize <= 65535)
    {
        header[1] = 126;
        header[2] = ((imgSize >> 8) & 255);
        header[3] = (imgSize & 255);
        moded = 4;
    }
    else
    {
        header[1] = 127;
        header[2] = ((imgSize >> 56) & 255);
        header[3] = ((imgSize >> 48) & 255);
        header[4] = ((imgSize >> 40) & 255);
        header[5] = ((imgSize >> 32) & 255);
        header[6] = ((imgSize >> 24) & 255);
        header[7] = ((imgSize >> 16) & 255);
        header[8] = ((imgSize >> 8) & 255);
        header[9] = (imgSize & 255);
        moded = 10;
    }
    if (sid != -1)
    {
        send(client_socket[sid], header, moded, 0);    // for websocket header
        send(client_socket[sid], data1, data1Size, 0); // for websocket data 1
        send(client_socket[sid], data2, data2Size, 0); // for websocket data 2
        return;
    }
    for (int i = 0; i < 30; i++)
    {
        if (client_socket[i] == 0 || this->ws_client_socket[i] == 0)
            continue;
        send(client_socket[i], header, moded, 0); // for websocket header
        send(client_socket[i], data1, data1Size, 0); // for websocket data 1
        send(client_socket[i], data2, data2Size, 0); // for websocket data 2
    }
}

void Websocket::decode(unsigned char *data, char *result)
{
    if(data[0] == 0) return;
    // decode a websocket frame which is less than 125 bytes
    int size = data[1] & 127;
    int index = 2;
    for (int i = 6; i < size + 6; i++)
    {
        result[i - 6] = data[i] ^ data[index++];
        if (index == 6)
        {
            index = 2;
        }
    }
}
#endif