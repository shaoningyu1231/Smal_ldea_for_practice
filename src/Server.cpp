#include "Server.h"
#include "Socket.h"
#include "Acceptor.h"
#include "Connection.h"
#include "ThreadPool.h"
#include "EventLoop.h"
#include <functional>
#include <unistd.h>

Server::Server(EventLoop *_loop) : mainReactor(_loop), acceptor(nullptr){    
    acceptor = new Acceptor(mainReactor);
    std::function<void(Socket*)> cb = std::bind(&Server::newConnection, this, std::placeholders::_1);
    acceptor->setNewConnectionCallback(cb);
    
    // Create a thread pool and sub-reactors
    // The number of threads is set to the number of hardware concurrency
    int size = std::thread::hardware_concurrency();
    thpool = new ThreadPool(size);
    for(int i = 0; i < size; ++i){
        subReactors.push_back(new EventLoop());
    }
    for(int i = 0; i < size; ++i){
        std::function<void()> sub_loop = std::bind(&EventLoop::loop, subReactors[i]);
        thpool->add(sub_loop);
    }
}

Server::~Server()
{
    delete acceptor;
    delete thpool;
}

// Handle new connections from clients
void Server::newConnection(Socket *serv_sock){
    if (serv_sock->getFd() == -1) {
        return; // If the socket is invalid, do nothing
    }
    int random = serv_sock->getFd() % subReactors.size(); // Randomly select a sub-reactor
    Connection *conn = new Connection(subReactors[random], serv_sock);
    std::function<void(int)> cb = std::bind(&Server::deleteConnection, this, std::placeholders::_1);
    conn->setDeleteConnectionCallback(cb);
    connections[serv_sock->getFd()] = conn;
}

// Handle disconnections from clients
void Server::deleteConnection(int sockfd){
    if (sockfd == -1) {
        return; // If the socket is invalid, do nothing
    }
    auto it = connections.find(sockfd);
    if (it == connections.end()) {
        return; // If the connection does not exist, do nothing
    }
    Connection *conn = connections[sockfd];
    connections.erase(sockfd);
    delete conn;
}
