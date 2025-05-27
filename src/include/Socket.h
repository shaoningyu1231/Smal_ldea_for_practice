#pragma once
#include <arpa/inet.h>

class InetAddress
{
    private:
        struct sockaddr_in addr;
        // socklen_t addr_len;
    public:
        InetAddress();
        InetAddress(const char* ip, uint16_t port);
        ~InetAddress();
        void setInetAddr(sockaddr_in _addr);
        sockaddr_in getAddr();
        // socklen_t getAddr_len();
        char* getIp();
        uint16_t getPort();
};

class Socket
{
private:
    int fd;
public:
    Socket();
    Socket(int _fd);
    ~Socket();

    void bind(InetAddress*);
    void listen();
    void setnonblocking();

    int accept(InetAddress*);
    void connect(InetAddress*);

    int getFd();
};