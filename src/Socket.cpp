#include "Socket.h"
#include "util.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <string.h>
#include <cerrno>

Socket::Socket() : fd(-1) {
    fd = socket(AF_INET, SOCK_STREAM, 0);
    errif(fd == -1, "socket create error");
}

Socket::Socket(int _fd) : fd(_fd) {
    errif(fd == -1, "socket create error");
}

Socket::~Socket() {
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
}

void Socket::bind(InetAddress *addr) {
    struct sockaddr_in addr_struct = addr->getAddr();
    // socklen_t addr_len = addr->getAddr_len();
    errif(::bind(fd, (sockaddr*)&addr_struct, sizeof(addr_struct)) == -1, "socket bind error");
}

void Socket::listen() {
    errif(::listen(fd, SOMAXCONN) == -1, "socket listen error");
}

void Socket::setnonblocking() {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

int Socket::accept(InetAddress *addr) {
    struct sockaddr_in addr_struct;
    int sockfd = -1;
    bzero(&addr_struct, sizeof(addr_struct));
    socklen_t addr_len = sizeof(addr_struct);
    if (fcntl(fd, F_GETFL) & O_NONBLOCK) {
        while (true) {
            sockfd = ::accept(fd, (sockaddr*)&addr_struct, &addr_len);
            if (sockfd == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
                // No connection yet, continue to wait
                continue;
            } else if (sockfd == -1) {
                errif(true, "socket accept error");
            } else {
                break; // Connection accepted
            }
        }
    }
    else{
        // For blocking sockets, we can directly call accept
        sockfd = ::accept(fd, (sockaddr*)&addr_struct, &addr_len);
        errif(sockfd == -1, "socket accept error");
    }
    addr->setInetAddr(addr_struct);
    return sockfd;
}

void Socket::connect(InetAddress *addr) {
    struct sockaddr_in addr_struct = addr->getAddr();
    socklen_t addr_len = sizeof(addr_struct);

    // For non-blocking sockets, we need to handle the connection attempt
    if (fcntl(fd, F_GETFL) & O_NONBLOCK) {
        while (true) {
            int ret = ::connect(fd, (sockaddr*)&addr_struct, addr_len);
            if (ret == 0) {
                break; // Connection successful
            } else if (ret == -1 && errno == EINPROGRESS) {
                continue; // Connection in progress
            } else if (ret == -1) {
                errif(true, "socket connect error");
            }
        }
    }
    // For blocking sockets, we can directly call connect
    else {
        errif(::connect(fd, (sockaddr*)&addr_struct, addr_len) == -1, "socket connect error");
    }
}

int Socket::getFd() {
    return fd;
}

InetAddress::InetAddress() {
    bzero(&addr, sizeof(addr));
}

InetAddress::InetAddress(const char* ip, uint16_t port) {
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    // addr_len = sizeof(addr);
}

InetAddress::~InetAddress() {
    // Destructor implementation (if needed)
}

void InetAddress::setInetAddr(sockaddr_in _addr) {
    addr = _addr;
    // addr_len = _addr_len;
}

sockaddr_in InetAddress::getAddr() {
    return addr;
}

char* InetAddress::getIp() {
    return inet_ntoa(addr.sin_addr);
}

uint16_t InetAddress::getPort() {
    return ntohs(addr.sin_port);
}
