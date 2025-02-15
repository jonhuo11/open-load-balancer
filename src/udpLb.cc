#include "udpLb.h"

#include "terminal.h"

LoadBalancerUDP::LoadBalancerUDP(const Config &cfg) : socket(), cfg(cfg), services(), running(false) {
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

    if (cfg.serviceCount < 1) {
        throw invalid_argument("Service count cannot be < 1");
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
            balanceStrategy = make_unique<RoundRobinBalance>(*this);
            break;
        default:
            balanceStrategy = make_unique<RandomBalance>(*this);
    }

    terminal = make_unique<Terminal>(*this);
}

LoadBalancerUDP::~LoadBalancerUDP() {
    cout << "\nShut down successfully" << endl;
}

void LoadBalancerUDP::stop() {
    running = false;
    terminal->stop();
}

void LoadBalancerUDP::start() {
    cout << "Starting load balancer..." << endl;
    promise<void> prom;
    future<void> fut = prom.get_future();

    // terminal runs on this thread, load balancer runs on separate
    thread lbThread([this, &prom] { loadBalancerThread(*this, prom); });
    terminal->start();

    lbThread.join(); // wait until both are stopped
    try {
        fut.get();
    } catch (const exception &e) {
        throw e;
    }
}

const ServerSocketUDP& LoadBalancerUDP::getSocket() const {
 return socket;
}

void LoadBalancerUDP::setRunning(bool b) {
    running.store(b, memory_order_release);
}

bool LoadBalancerUDP::isRunning() const {
    return running.load(memory_order_relaxed);
}

void LoadBalancerUDP::routePacket(const PacketUDP& packet) {
    balanceStrategy->routePacket(packet);
}


void loadBalancerThread(LoadBalancerUDP& lb, promise<void> &exceptionPromise) {
    try {
        FileDescriptor epoll(epoll_create1(0));

        const size_t MAX_EVENTS = 1024;
        struct epoll_event event, events[MAX_EVENTS];  // MAX_EVENTS epoll events batched at a time
        int epollTimeout = 0;                          // NOTE: -1 will BLOCK the thread!!!

        PacketUDP packet = PacketUDP();  // used for reading in data
        struct sockaddr_in clientAddr;   // used for reading client ip/port
        socklen_t clientLen = sizeof(clientAddr);

        event.events = EPOLLIN;
        event.data.fd = lb.getSocket().getSocket();
        if (epoll_ctl(epoll.getFd(), EPOLL_CTL_ADD, lb.getSocket().getSocket(), &event) < 0) throw runtime_error("epoll_ctl < 0");

        lb.setRunning(true);
        while (lb.isRunning()) {
            int nfds = epoll_wait(epoll.getFd(), events, MAX_EVENTS, epollTimeout);
            if (nfds < 0) {
                throw runtime_error("epoll_wait failed");
            }

            // does not execute if nfds < 1
            for (int i = 0; i < nfds; ++i) {
                if (events[i].events & EPOLLIN) {  // Data is available to read
                    ssize_t nBytesRead = recvfrom(lb.getSocket().getSocket(), packet.data, PacketUDP::BUFFER_SIZE, 0, (struct sockaddr *)&clientAddr, &clientLen);
                    packet.sender.port = clientAddr.sin_port;  // TODO: can we directly copy into this field for speed?
                    packet.sender.ip = clientAddr.sin_addr.s_addr;

                    if (nBytesRead < 0) {  // TODO: write some error somehow?
                        continue;
                    }

                    lb.routePacket(packet);
                }
            }
        }
    } catch (const exception &e) {
        exceptionPromise.set_exception(make_exception_ptr(e));
        return;
    }
    exceptionPromise.set_value();
}