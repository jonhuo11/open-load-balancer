#include "udpSockets.h"

SocketUDP::SocketUDP() {
    socket = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (socket < 0) throw exception();
}

SocketUDP::~SocketUDP() {
    if (canCloseSocket()) close(socket);
}

int SocketUDP::getSocket() const {
    return socket;
}

ServiceSocketUDP::ServiceSocketUDP(const char *ip, uint16_t port) : socket() {
    memset(&destAddr, 0, sizeof(destAddr));    // clear struct fields to 0
    destAddr.sin_family = AF_INET;             // IPv4
    destAddr.sin_port = htons(port);           // Port number
    destAddr.sin_addr.s_addr = inet_addr(ip);  // Localhost IP address
}

ServiceSocketUDP::~ServiceSocketUDP() {}

int ServiceSocketUDP::send(const char *message) {
    return sendto(socket.getSocket(), message, strlen(message), 0, (struct sockaddr *)&destAddr, sizeof(destAddr));
}

void ServerSocketUDP::bind(const sockaddr *addr, const socklen_t len) {
}