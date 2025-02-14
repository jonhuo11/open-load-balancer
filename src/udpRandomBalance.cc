#include "udpLb.h"

LoadBalancerUDP::RandomBalance::RandomBalance(const LoadBalancerUDP &lb) : BalanceStrategy(lb), gen(rdev()), distr(0, lb.services.size() - 1) {}

void LoadBalancerUDP::RandomBalance::routePacket(PacketUDP& p) {
	
}

void LoadBalancerUDP::RandomBalance::serviceUp(size_t serviceKey) {
}

void LoadBalancerUDP::RandomBalance::serviceDown(size_t) {
}