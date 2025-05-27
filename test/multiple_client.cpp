#include <iostream>
#include <unistd.h>
#include <string.h>
#include <functional>
#include "src/util.h"
#include "src/Buffer.h"
#include "src/Socket.h"
#include "src/ThreadPool.h"

using namespace std;

void oneClient(int msgs, int wait){
    Socket *sock = new Socket();
    InetAddress *addr = new InetAddress("127.0.0.1", 1234);
    sock->connect(addr);
    int sockfd = sock->getFd();

    Buffer *sendBuffer = new Buffer();
    Buffer *readBuffer = new Buffer();

    // Emulate a delay before sending messages
    sleep(wait);
    int count = 0;
    while(count < msgs){
        sendBuffer->setBuf("I'm client!");
        ssize_t write_bytes = write(sockfd, sendBuffer->c_str(), sendBuffer->size());
        if(write_bytes == -1){
            printf("socket already disconnected, can't write any more!\n");
            break;
        }
        int already_read = 0;
        char buf[1024];    //这个buf大小无所谓
        while(true){
            bzero(&buf, sizeof(buf));
            ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
            if(read_bytes > 0){
                readBuffer->append(buf, read_bytes);
                already_read += read_bytes;
            } else if(read_bytes == 0){         //EOF
                printf("server disconnected!\n");
                exit(EXIT_SUCCESS);
            }
            if(already_read >= sendBuffer->size()){
                printf("count: %d, message from server: %s\n", count++, readBuffer->c_str());
                break;
            } 
        }
        readBuffer->clear();
    }
    delete addr;
    delete sock;
    delete readBuffer;
    delete sendBuffer;
}

int main(int argc, char *argv[]) {
    int threads = 100;
    int msgs = 100;
    int wait = 0;
    int o = -1;
    const char *optstring = "t:m:w:";

    while((o = getopt(argc, argv, optstring)) != -1){
        switch(o){
            case 't':
                threads = atoi(optarg);
                break;
            case 'm':
                msgs = atoi(optarg);
                break;
            case 'w':
                wait = atoi(optarg);
                break;
            default:
                printf("Usage: %s [-t threads] [-m msgs] [-w wait]\n", argv[0]);
                return -1;
        }
    }

    ThreadPool *pool = new ThreadPool(threads);
    for(int i = 0; i < threads; ++i){
        pool->add(std::bind(oneClient, msgs, wait));
    }
    delete pool;

    return 0;
}
