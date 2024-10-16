#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

using namespace std;

class ClientSocketUDP {
    int socket;
    struct sockaddr_in dest_addr;
public:
    ClientSocketUDP(const char* ip, uint16_t port) {
        socket = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (socket < 0) {
            throw exception();
        }

        memset(&dest_addr, 0, sizeof(dest_addr));             // clear struct fields to 0
        dest_addr.sin_family = AF_INET;                       // IPv4
        dest_addr.sin_port = htons(port);                     // Port number
        dest_addr.sin_addr.s_addr = inet_addr(ip);   // Localhost IP address
    }
    ~ClientSocketUDP() {
        close(socket);
    }

    int send(const char* message) {
        int bytes_sent = sendto(socket, message, strlen(message), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
        return bytes_sent;
    }
};