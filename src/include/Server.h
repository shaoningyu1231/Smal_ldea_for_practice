#pragma once

class EventLoop;
class Socket;
class Acceptor;
class Connection;
class ThreadPool;
#include <map>
#include <vector>
class Server
{
private:
    EventLoop *mainReactor;
    Acceptor *acceptor;
    std::map<int, Connection*> connections;
    std::vector<EventLoop*> subReactors;
    ThreadPool *thpool;
public:
    Server(EventLoop*);
    ~Server();

    void handleReadEvent(int);
    void newConnection(Socket *serv_sock);
    void deleteConnection(int sockfd);
};
