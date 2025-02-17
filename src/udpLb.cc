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
        services.upsert(i, make_unique<ServiceSocketUDP>("127.0.0.1", cfg.servicePorts[i]));  // localhost sockets
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
    if (running.load()) {
        stop();
    }
    cout << "\nShut down successfully" << endl;
}

void LoadBalancerUDP::stop() {
    //cout << "got to stopping lb" << endl;
    unique_lock<shared_mutex> lock(mutex); // TODO: not sure if this needed since running is atomic anyways
    //cout << "completed lock acquisition" << endl;
    if (!running.load()) { return; }
    //cout << "completed running check" << endl;
    running.store(false);
    //cout << "done quitting in lb" << endl;
}

void LoadBalancerUDP::start() {
    cout << "Starting load balancer..." << endl;
    unique_lock<shared_mutex> lock(mutex);
    if (running.load()) {
        return;
    }
    running.store(true);
    lock.unlock();
    promise<void> prom;
    future<void> fut = prom.get_future();
    thread lbThread;

    lbThread = thread([this, &prom] { loadBalancerSingleThreaded(*this, prom); });
    terminal->start();
    //cout << "terminal done" << endl;

    if (lbThread.joinable()) {
        //cout << "waiting to join" << endl;
        lbThread.join();
    }
    try {
        fut.get();
    } catch (const exception &e) {
        throw e;
    }
}

const ServerSocketUDP& LoadBalancerUDP::getSocket() const {
    shared_lock<shared_mutex> lock(mutex);
    return socket;
}

bool LoadBalancerUDP::isRunning() const { // TODO: revisit this once I figure out if this needs a mutex or not
    shared_lock<shared_mutex> lock(mutex);
    return running.load();
}

void LoadBalancerUDP::routePacket(const PacketUDP& packet) {
    unique_lock<shared_mutex> lock(mutex);
    balanceStrategy->routePacket(packet);
}

string LoadBalancerUDP::listServices() const {
    shared_lock<shared_mutex> lock(mutex);
    auto serviceKeys = services.keys();
    ostringstream oss;
    if (!serviceKeys.empty()) {
        copy(serviceKeys.begin(), serviceKeys.end() - 1, ostream_iterator<size_t>(oss, " "));
        oss << serviceKeys.back(); // Add the last element without a trailing space
    }
    return oss.str();
}

void LoadBalancerUDP::serviceUp(const char* ip, uint16_t port) {
    unique_lock<shared_mutex> lock(mutex);
}

void LoadBalancerUDP::serviceDown (const size_t& serviceKey) {
    unique_lock<shared_mutex> lock(mutex);
    balanceStrategy->serviceDown(serviceKey);
}


void loadBalancerSingleThreaded(LoadBalancerUDP& lb, promise<void> &exceptionPromise) {
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