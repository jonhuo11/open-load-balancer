#include "udpLb.h"

LoadBalancerUDP::RandomBalance::RandomBalance(LoadBalancerUDP &lb) : BalanceStrategy(lb), gen(rdev()), distr(0, lb.services.size() - 1) {}

size_t LoadBalancerUDP::RandomBalance::calculateDestinationServiceForPacket(PacketUDP &p) {
    (void)p;
    size_t destServiceIndex = (size_t)distr(gen);
    return destServiceIndex;
}

void LoadBalancerUDP::RandomBalance::serviceUp(unique_ptr<ServiceSocketUDP>) {
}

void LoadBalancerUDP::RandomBalance::serviceDown(size_t) {
}