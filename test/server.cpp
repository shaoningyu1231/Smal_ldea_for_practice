#include "src/EventLoop.h"
#include "src/Server.h"

int main() {
    EventLoop *loop = new EventLoop();
    Server *server = new Server(loop);
    loop->loop();
    delete server; // Clean up the server object
    delete loop;   // Clean up the event loop object
    return 0;
}