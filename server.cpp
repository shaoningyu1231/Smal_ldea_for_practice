#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <vector>
#include "util.h"
#include "Epoll.h"
#include "InetAddress.h"
#include "Socket.h"

# define MAX_EVENTS 1024
# define READ_BUFFER 1024

// Function to set a file descriptor to non-blocking mode
void setnonblocking(int fd) {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

void handleReadEvent(int sockfd) {
    char buf[READ_BUFFER];
    while (true) { // Read until no more data is available
        bzero(&buf, sizeof(buf));
        ssize_t bytes_read = read(sockfd, buf, sizeof(buf));
        if (bytes_read > 0) {
            printf("message from client fd %d: %s\n", sockfd, buf);
            write(sockfd, buf, bytes_read); // Echo back to client
        } else if (bytes_read == -1 && errno == EINTR) { // Interrupted system call
            continue;
        } else if (bytes_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) { // No more data
            printf("No more data to read from client fd %d\n", sockfd);
            break; // Exit the loop to avoid busy waiting
        } else if (bytes_read == 0) { // Client disconnected
            printf("client fd %d disconnected!\n", sockfd);
            close(sockfd);
            break;
        } else { // Error occurred
            perror("socket read error");
            close(sockfd);
            break;
        }
    }
}

int main(){
    // Create a socket
    Socket *serv_sock = new Socket();
    InetAddress *serv_addr = new InetAddress("127.0.0.1", 8888);
    serv_sock->bind(serv_addr);
    serv_sock->listen();
    Epoll *ep = new Epoll();
    serv_sock->setnonblocking();
    ep->addFd(serv_sock->getFd(), EPOLLIN | EPOLLET);
    while (true) {
        std::vector<epoll_event> events = ep->poll();
        int nfds = events.size();
        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.fd == serv_sock->getFd()) { // New client connection
                InetAddress *clnt_addr = new InetAddress(); // Memory leak! No delete
                Socket *clnt_sock = new Socket(serv_sock->accept(clnt_addr)); // Memory leak! No delete
                printf("new client fd %d! IP: %s Port: %d\n", clnt_sock->getFd(), inet_ntoa(clnt_addr->addr.sin_addr), ntohs(clnt_addr->addr.sin_port));
                clnt_sock->setnonblocking();
                ep->addFd(clnt_sock->getFd(), EPOLLIN | EPOLLET);
            } else if (events[i].events & EPOLLIN) { // Read event
                handleReadEvent(events[i].data.fd);
            } else { // Other events
                printf("something else happened\n");
            }
        }
    }
    delete serv_sock;
    delete serv_addr;
    return 0;
}