# include <iostream>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <string.h>
# include <unistd.h>
# include "src/util.h"
# include "src/Buffer.h"

#define BUFFER_SIZE 1024

// This is a echo client program that connects to a server, sends messages, and receives responses.
// The client uses a socket to communicate with the server over TCP/IP.
int main() {
    Socket *sock = new Socket();
    InetAddress *addr = new InetAddress("127.0.0.1", 1234);
    sock->connect(addr);
    int sockfd = sock->getFd();
    
    Buffer *sendBuffer = new Buffer();
    Buffer *readBuffer = new Buffer();

    while (true) {
        sendBuffer->getline();
        ssize_t write_bytes = write(sockfd, sendBuffer->c_str(), sendBuffer->size());
        if (write_bytes == -1) {
            std::cerr << "socket already disconnected, can't write any more!" << std::endl;
            break;
        }
        int already_read = 0;
        char buf[BUFFER_SIZE]; // Buffer to read data from the server
        while (true) {
            bzero(&buf, sizeof(buf)); // Clear the buffer before reading
            ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
            if (read_bytes > 0) {
                readBuffer->append(buf, read_bytes);
                already_read += read_bytes;
            } else if (read_bytes == 0) { // EOF
                std::cout << "server disconnected!" << std::endl;
                exit(EXIT_SUCCESS);
            }
            if (already_read >= sendBuffer->size()) {
                std::cout << "message from server: " << readBuffer->c_str() << std::endl;
                break;
            }
        }
        readBuffer->clear(); // Clear the read buffer for the next iteration
    }
    delete addr; // Clean up the address object
    delete sock; // Clean up the socket object
    return 0;
}