#include <iostream>
#include <cstring>      // For memset
#include <sys/types.h> // For data types
#include <sys/socket.h> // For socket functions
#include <netinet/in.h> // For sockaddr_in
#include <unistd.h>    // For close()

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: client [port]" << std::endl;
        return 1;
    }
    uint16_t port;
    try {
        port = std::stoi(argv[1]);
    } catch (...) {return 1;}

    // Create a UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error creating socket." << std::endl;
        return 1;
    }

    // Define server address
    struct sockaddr_in serverAddr, clientAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Accept any incoming address
    serverAddr.sin_port = htons(port); // Set port number (e.g., 12345)

    // Bind the socket to the address
    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error binding socket." << std::endl;
        close(sockfd);
        return 1;
    }

    std::cout << "UDP server listening on port " << port << std::endl;

    // Buffer for incoming data
    char buffer[1024];
    socklen_t clientAddrLen = sizeof(clientAddr);

    // Receive data in an infinite loop
    while (true) {
        ssize_t bytesReceived = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                                         (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (bytesReceived < 0) {
            std::cerr << "Error receiving data." << std::endl;
            break;
        }

        // Null-terminate the received data and print it
        buffer[bytesReceived] = '\0'; // Null-terminate the string
        std::cout << "Received: " << buffer << std::endl;
    }

    // Close the socket
    close(sockfd);
    return 0;
}
