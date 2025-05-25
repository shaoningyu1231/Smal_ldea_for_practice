#include "Connection.h"
#include "Socket.h"
#include "Channel.h"
#include <unistd.h>
#include <string.h>
#include <cstdio>
#include <errno.h>
#include "util.h"

#define READ_BUFFER 1024

Connection::Connection(EventLoop *_loop, Socket *_sock) : loop(_loop), sock(_sock), channel(nullptr), readBuffer(nullptr) {
    channel = new Channel(loop, sock->getFd());
    std::function<void()> cb = std::bind(&Connection::echo, this, sock->getFd());
    channel->setCallback(cb);
    channel->enableReading();
    channel->useET();
    readBuffer = new Buffer();
}

Connection::~Connection() {
    delete channel;
    delete sock;
    delete readBuffer;
}

void Connection::echo(int sockfd) {
    char buf[READ_BUFFER];
    while (true) {    
        bzero(&buf, sizeof(buf));

        // Read data from the socket
        ssize_t bytes_read = read(sockfd, buf, sizeof(buf));
        if (bytes_read > 0) {
            readBuffer->append(buf, bytes_read); 
        } else if (bytes_read == -1 && errno == EINTR) {  
            // Interrupted system call, continue reading
            printf("continue reading");
            continue;
        } else if (bytes_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) { 
            // Non-blocking IO, this condition indicates that all data has been read
            printf("finish reading once, errno: %d\n", errno);
            printf("message from client fd %d: %s\n", sockfd, readBuffer->c_str());
            errif(write(sockfd, readBuffer->c_str(), readBuffer->size()) == -1, "socket write error");
            readBuffer->clear();
            break;
        } else if (bytes_read == 0) { 
            printf("EOF, client fd %d disconnected\n", sockfd);
            deleteConnectionCallback(sock);
            break;
        }
    }
}

void Connection::setDeleteConnectionCallback(std::function<void(Socket*)> _cb) {
    deleteConnectionCallback = _cb;
}

void Connection::send(int sockfd){
    char buf[readBuffer->size()];
    strcpy(buf, readBuffer->c_str());
    int data_size = readBuffer->size();
    int already_send = 0;
    while (already_send < data_size) {
        ssize_t bytes_send = write(sockfd, buf + already_send, data_size - already_send);
        if (bytes_send > 0) {
            already_send += bytes_send;
        } else if (bytes_send == -1 && errno == EINTR) {
            continue; // Interrupted system call, continue sending
        } else if (bytes_send == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
            break; // Non-blocking IO, break the loop
        } else {
            printf("send error\n");
            break;
        }
    }
}
