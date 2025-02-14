#include "udpLb.h"

LoadBalancerUDP::RandomBalance::RandomBalance(LoadBalancerUDP &lb) : BalanceStrategy(lb), gen(rdev()), distr(0, lb.services.size() - 1) {}

void LoadBalancerUDP::RandomBalance::routePacket(PacketUDP& p) {
	
}

void LoadBalancerUDP::RandomBalance::serviceUp(unique_ptr<ServiceSocketUDP>&&) {
}

void LoadBalancerUDP::RandomBalance::serviceDown(size_t) {
}