#include "udpLb.h"

atomic<bool> stop;
void handleSignal(int sig) {
    stop = true;
}

LoadBalancerUDP::LoadBalancerUDP(const Config &cfg, const vector<unique_ptr<ServiceSocketUDP>> &services) : socket(), cfg(cfg), services(services) {
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;                      // IPv4
    serverAddr.sin_port = htons(cfg.listenPort);          // Port number
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Localhost IP address

    try {
        socket.bind((struct sockaddr *)&serverAddr, sizeof(serverAddr));
    } catch (runtime_error &) {
        // TODO: invalidate the lb
        cerr << "The load balancer could not bind to the socket on port " << cfg.listenPort << endl;
        throw;
    }

    // pick strategy
    switch (cfg.mode) {
        case 0:  // random
            balanceStrategy = make_unique<RandomBalance>(*this);
            break;
        case 1:  // rr
            balanceStrategy = make_unique<RoundRobinServiceAssignmentBalance>(*this);
            break;
        default:
            balanceStrategy = make_unique<RandomBalance>(*this);
    }
}

LoadBalancerUDP::RandomBalance::RandomBalance(LoadBalancerUDP &lb) : BalanceStrategy(lb), gen(rdev()), distr(0, lb.services.size() - 1) {}

size_t LoadBalancerUDP::RandomBalance::calculateDestinationServiceForPacket(PacketUDP &p) {
    size_t destServiceIndex = (size_t)distr(gen);
    return destServiceIndex;
}

LoadBalancerUDP::RoundRobinServiceAssignmentBalance::RoundRobinServiceAssignmentBalance(LoadBalancerUDP &lb) : BalanceStrategy(lb) {}

size_t LoadBalancerUDP::RoundRobinServiceAssignmentBalance::calculateDestinationServiceForPacket(PacketUDP &p) {
    // is it a new client?
    auto destServiceIter = clientServiceMap.find(p.sender);
    if (destServiceIter == clientServiceMap.end()) {
        // assign to the next round
        clientServiceMap[p.sender] = (round_i++) % lb.services.size();
        destServiceIter = clientServiceMap.find(p.sender);
    }

    // grab the client
    size_t destServiceIndex = destServiceIter->second;
    return destServiceIndex;
}

void LoadBalancerUDP::main() {
    signal(SIGINT, handleSignal);
    FileDescriptor epoll(epoll_create1(0));

    const size_t MAX_EVENTS = 128;
    struct epoll_event event, events[MAX_EVENTS];  // 128 epoll events batched at a time

    PacketUDP packet = PacketUDP();  // used for reading in data
    struct sockaddr_in clientAddr;   // used for reading client ip/port
    socklen_t clientLen = sizeof(clientAddr);

    event.events = EPOLLIN;
    event.data.fd = socket.getSocket();
    if (epoll_ctl(epoll.getFd(), EPOLL_CTL_ADD, socket.getSocket(), &event) < 0) throw runtime_error("epoll_ctl < 0");

    while (!stop) {
        int nfds = epoll_wait(epoll.getFd(), events, MAX_EVENTS, -1);  // number of events
        if (nfds < 0) {
            throw runtime_error("epoll_wait failed");
        }

        // does not execute if nfds < 1
        for (int i = 0; i < nfds; ++i) {
            if (events[i].events && EPOLLIN) {  // Data is available to read
                ssize_t nBytesRead = recvfrom(socket.getSocket(), packet.data, PacketUDP::BUFFER_SIZE, 0, (struct sockaddr *)&clientAddr, &clientLen);
                packet.sender.port = clientAddr.sin_addr.s_addr;  // TODO: can we directly copy into this field for speed?
                packet.sender.ip = clientAddr.sin_port;

                if (nBytesRead < 0) {
                    cerr << "recvfrom server socket failed" << endl;
                    continue;
                }
                size_t destServiceIndex = balanceStrategy->calculateDestinationServiceForPacket(packet);
                services[destServiceIndex]->send(packet.data);
            }
        }
    }
    cout << "Exiting." << endl;
}