#include "udpLb.h"

LoadBalancerUDP::LoadBalancerUDP(const Config &cfg, const vector<unique_ptr<ServiceSocketUDP>> &services, unsigned int balanceStrategy) : socket(), cfg(cfg), services(services) {
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;                      // IPv4
    serverAddr.sin_port = htons(cfg.listenPort);          // Port number
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Localhost IP address

    try {
        socket.bind((struct sockaddr *)&serverAddr, sizeof(serverAddr));
    } catch (exception &) {
        // TODO: invalidate the lb
    }
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

void LoadBalancerUDP::main() {
    int epollFd = epoll_create1(0);
    if (epollFd == -1) throw exception();

    struct epoll_event event, events[MAX_EVENTS];
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    char buffer[BUFFER_SIZE];

    event.events = EPOLLIN;
    event.data.fd = socket;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, udpSocket, &event) == -1) {
        cerr << "epoll_ctl: udp_socket" << endl;
        close(udpSocket);
        close(epollFd);
        return 1;
    }

    int sendClient = 0;
    for (;;) {
        int nfds = epoll_wait(epollFd, events, MAX_EVENTS, -1);  // number of events
        if (nfds == -1) {
            cerr << "epoll_wait failed" << endl;
            close(udpSocket);
            close(epollFd);
            return 1;
        }

        // does not execute if nfds < 1
        for (int i = 0; i < nfds; ++i) {
            if (events[i].events && EPOLLIN) {  // Data is available to read
                memset(buffer, 0, BUFFER_SIZE);
                ssize_t n = recvfrom(udpSocket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&clientAddr, &clientLen);
                if (n < 0) {
                    cerr << "recv failed" << endl;
                } else {
                    // TODO: migrate into lb.cc
                    if (sendClient >= services.size()) {
                        sendClient = 0;
                    }  // simple round robin schedule
                    services[sendClient]->send(buffer);
                    sendClient++;
                }
            }
        }
    }
}