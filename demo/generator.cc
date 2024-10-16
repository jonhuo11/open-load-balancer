#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: generator [id] [send port] [message count]" << std::endl;
        return 1;
    }
    string id;
    uint16_t port; // destination port
    int message_count;
    try {
        id = argv[1];
        port = std::stoi(argv[2]);
        message_count = std::stoi(argv[3]);
    } catch (...) {return 1;}

    int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket < 0) {
        cerr << "Error creating socket" << endl;
        return -1;
    }

    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;                       // IPv4
    dest_addr.sin_port = htons(port);                     // Port number
    dest_addr.sin_addr.s_addr = inet_addr("127.0.0.1");   // Localhost IP address

    string message = "Hello from generator " + id;
    const char* sendable_message = message.c_str();
    for (int i = 0; i < message_count; i++) {
        sendto(udp_socket, sendable_message, strlen(sendable_message), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
        sleep(2);
    }

    close(udp_socket);
    return 0;
}