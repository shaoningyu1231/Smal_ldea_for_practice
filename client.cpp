# include <iostream>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <string.h>
# include <unistd.h>
# include "src/util.h"

#define BUFFER_SIZE 1024

// This is a echo client program that connects to a server, sends messages, and receives responses.
// The client uses a socket to communicate with the server over TCP/IP.
int main() {
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
    serv_addr.sin_port = htons(1234);
    
    // Connect to the server
    if (connect(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("socket connect error");
        close(sockfd);
        return -1;
    }

    while (true) {
        char buf[BUFFER_SIZE];  // Ensure this buffer is large enough for your needs
        bzero(&buf, sizeof(buf));
        
        // Read user input
        std::cin >> buf;
        
        // Send data to the server
        ssize_t write_bytes = write(sockfd, buf, sizeof(buf));
        if (write_bytes == -1) {
            std::cerr << "socket already disconnected, can't write any more!" << std::endl;
            break;
        }
        
        bzero(&buf, sizeof(buf));
        
        // Read response from the server
        ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
        if (read_bytes > 0) {
            std::cout << "message from server: " << buf << std::endl;
        } else if (read_bytes == 0) {
            std::cout << "server socket disconnected!" << std::endl;
            break;
        } else if (read_bytes == -1) {
            close(sockfd);
            perror("socket read error");
            break;
        }
    }
    close(sockfd);
    return 0;
}
