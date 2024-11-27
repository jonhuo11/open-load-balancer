#include "lb_udp.h"

ServiceSocketUDP::ServiceSocketUDP(const char *ip, uint16_t port) {
    socket = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (socket < 0) {
        throw exception();
    }

    memset(&destAddr, 0, sizeof(destAddr));    // clear struct fields to 0
    destAddr.sin_family = AF_INET;             // IPv4
    destAddr.sin_port = htons(port);           // Port number
    destAddr.sin_addr.s_addr = inet_addr(ip);  // Localhost IP address
}

ServiceSocketUDP::~ServiceSocketUDP() { close(socket); }

int ServiceSocketUDP::send(const char *message) {
    int bytesSent = sendto(socket, message, strlen(message), 0, (struct sockaddr *)&destAddr, sizeof(destAddr));
    return bytesSent;
}

LoadBalancerUDP::RandomBalance::RandomBalance(LoadBalancerUDP &lb) : BalanceStrategy(lb), gen(rdev()), distr(0, lb.services.size() - 1) {}

size_t LoadBalancerUDP::RandomBalance::calculateDestinationServiceForPacket(Packet &p) {
    size_t destServiceIndex = (size_t)distr(gen);
    // TODO: send the packet data to that client
    return 0;
}

size_t LoadBalancerUDP::RoundRobinServiceAssignmentBalance::calculateDestinationServiceForPacket(Packet &p) {
    // is it a new client?
    auto destServiceIter = clientServiceMap.find(p.sender);
    if (destServiceIter == clientServiceMap.end()) {
        // assign to the next round
        clientServiceMap[p.sender] = (round_i++) % lb.services.size();
        destServiceIter = clientServiceMap.find(p.sender);
    }

    // grab the client
    size_t destServiceIndex = destServiceIter->second;

    // TODO: send the packet data to that client
    return 0;
}