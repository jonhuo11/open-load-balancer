#include "udpLb.h"

#include "terminal.h"

LoadBalancerUDP::LoadBalancerUDP(const Config &cfg) : socket(), cfg(cfg), services(cfg.serviceCount), running(false) {
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

    // set up services
    for (int i = 0; i < cfg.serviceCount; ++i) {
        services[i] = make_unique<ServiceSocketUDP>("127.0.0.1", cfg.servicePorts[i]);  // localhost sockets
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

    terminal = make_unique<Terminal>(*this);
}

LoadBalancerUDP::~LoadBalancerUDP() {
    cout << "\nShut down successfully" << endl;
}

void LoadBalancerUDP::main(promise<void> &exceptionPromise) {
    try {
        FileDescriptor epoll(epoll_create1(0));

        const size_t MAX_EVENTS = 128;
        struct epoll_event event, events[MAX_EVENTS];  // 128 epoll events batched at a time
        int epollTimeout = 0;                          // NOTE: -1 will BLOCK the thread!!!

        PacketUDP packet = PacketUDP();  // used for reading in data
        struct sockaddr_in clientAddr;   // used for reading client ip/port
        socklen_t clientLen = sizeof(clientAddr);

        event.events = EPOLLIN;
        event.data.fd = socket.getSocket();
        if (epoll_ctl(epoll.getFd(), EPOLL_CTL_ADD, socket.getSocket(), &event) < 0) throw runtime_error("epoll_ctl < 0");

        running = true;
        while (running) {
            int nfds = epoll_wait(epoll.getFd(), events, MAX_EVENTS, epollTimeout);
            if (nfds < 0) {
                throw runtime_error("epoll_wait failed");
            }

            // does not execute if nfds < 1
            for (int i = 0; i < nfds; ++i) {
                if (events[i].events && EPOLLIN) {  // Data is available to read
                    ssize_t nBytesRead = recvfrom(socket.getSocket(), packet.data, PacketUDP::BUFFER_SIZE, 0, (struct sockaddr *)&clientAddr, &clientLen);
                    packet.sender.port = clientAddr.sin_port;  // TODO: can we directly copy into this field for speed?
                    packet.sender.ip = clientAddr.sin_addr.s_addr;

                    if (nBytesRead < 0) {  // TODO: write some error somehow?
                        continue;
                    }
                    size_t destServiceIndex = balanceStrategy->calculateDestinationServiceForPacket(packet);
                    services[destServiceIndex]->send(packet.data);
                }
            }
        }
    } catch (const exception &e) {
        exceptionPromise.set_exception(make_exception_ptr(e));
        return;
    }
    exceptionPromise.set_value();
}

void LoadBalancerUDP::stop() {
    running = false;
    terminal->stop();
}

void LoadBalancerUDP::start() {
    cout << "Starting load balancer..." << endl;
    promise<void> prom;
    future<void> fut = prom.get_future();
    thread lbThread(std::bind(&LoadBalancerUDP::main, this, std::ref(prom)));
    terminal->start();
    lbThread.join();
    try {
        fut.get();
    } catch (const exception &e) {
        throw e;
    }
}
