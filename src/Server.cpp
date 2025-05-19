#include "Server.h"
#include "Socket.h"
#include "Acceptor.h"
#include "Connection.h"
#include <functional>

Server::Server(EventLoop *_loop) : loop(_loop), acceptor(nullptr){    
    acceptor = new Acceptor(loop);
    std::function<void(Socket*)> cb = std::bind(&Server::newConnection, this, std::placeholders::_1);
    acceptor->setNewConnectionCallback(cb);
}

Server::~Server()
{
    delete acceptor;
}

// Handle new connections from clients
void Server::newConnection(Socket *serv_sock){
    Connection *conn = new Connection(loop, serv_sock);
    std::function<void(Socket*)> cb = std::bind(&Server::deleteConnection, this, std::placeholders::_1);
    conn->setDeleteConnectionCallback(cb);
    connections[serv_sock->getFd()] = conn;
}

// Handle disconnections from clients
void Server::deleteConnection(Socket *serv_sock){
    Connection *conn = connections[serv_sock->getFd()];
    connections.erase(serv_sock->getFd());
    delete conn;
}
