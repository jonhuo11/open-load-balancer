#include "udpLb.h"

LoadBalancerUDP::RoundRobinServiceAssignmentBalance::RoundRobinServiceAssignmentBalance(LoadBalancerUDP &lb) : BalanceStrategy(lb) {}

void LoadBalancerUDP::RoundRobinServiceAssignmentBalance::routePacket(PacketUDP &p) {

}


void LoadBalancerUDP::RoundRobinServiceAssignmentBalance::serviceUp(unique_ptr<ServiceSocketUDP>&& newService) {
    
}

void LoadBalancerUDP::RoundRobinServiceAssignmentBalance::serviceDown(size_t downedServiceIndex) {
    // migrate all existing clients from the downed service to new services


    // remove the downed service
}
