# include <stdio.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <string.h>
# include <unistd.h>
# include <fcntl.h>
# include <sys/epoll.h>
# include <errno.h>
# include "util.h"

# define MAX_EVENTS 1024
# define READ_BUFFER 1024

// Function to set a file descriptor to non-blocking mode
void setnonblocking(int fd) {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

int main(){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket create error");
        return -1;
    }

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    
    // Set the server address to localhost (for testing purposes)
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(8888);

    // Bind the socket to the address
    if (bind(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("socket bind error");
        close(sockfd);
        return -1;
    }

    // Listen for incoming connections
    if (listen(sockfd, SOMAXCONN) == -1) {
        perror("socket listen error");
        close(sockfd);
        return -1;
    }

    // Create an epoll instance for multiple client handling
    int epfd = epoll_create1(0);
    if (epfd == -1) {
        perror("epoll create error");
        close(sockfd);
        return -1;
    }
    struct epoll_event events[MAX_EVENTS], ev;
    bzero(&events, sizeof(events));
    bzero(&ev, sizeof(ev));
    ev.data.fd = sockfd;
    
    // Set the socket to non-blocking mode
    setnonblocking(sockfd);
    ev.events = EPOLLIN | EPOLLET; // Edge-triggered mode
    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

    while (true) {
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll wait error");
            break;
        }
        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.fd == sockfd) { // New client connection
                
                // Accept the new client connection and initialize the client address
                struct sockaddr_in clnt_addr;
                bzero(&clnt_addr, sizeof(clnt_addr));
                socklen_t clnt_addr_len = sizeof(clnt_addr);

                int clnt_sockfd = accept(sockfd, (sockaddr*)&clnt_addr, &clnt_addr_len);
                if (clnt_sockfd == -1) {
                    perror("socket accept error");
                    continue;
                }
                printf("new client fd %d! IP: %s Port: %d\n", clnt_sockfd, inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));

                // resume the epoll event for the new client
                bzero(&ev, sizeof(ev));
                ev.data.fd = clnt_sockfd;
                ev.events = EPOLLIN | EPOLLET; // Edge-triggered mode
                setnonblocking(clnt_sockfd);
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sockfd, &ev);
            } else if (events[i].events & EPOLLIN)  { // Handle client data
                char buf[READ_BUFFER];
                bzero(&buf, sizeof(buf));
                
                ssize_t read_bytes = read(events[i].data.fd, buf, sizeof(buf));
                if (read_bytes > 0) {
                    printf("message from client fd %d: %s\n", events[i].data.fd, buf);
                    write(events[i].data.fd, buf, read_bytes); // Echo back to client
                } else if (read_bytes == 0) {
                    printf("client fd %d disconnected!\n", events[i].data.fd);
                    close(events[i].data.fd);
                } else if (read_bytes == -1) {
                    close(events[i].data.fd);
                    perror("socket read error");
                }
            } else { // Handle other events
                printf("something else happened\n");
            }
        }
    }
    close(sockfd);
    close(epfd);
    return 0;
}