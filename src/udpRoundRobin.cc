#include "udpLb.h"

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

void LoadBalancerUDP::RoundRobinServiceAssignmentBalance::serviceUp(unique_ptr<ServiceSocketUDP> newService) {
    lb.services.emplace_back(newService);
}

void LoadBalancerUDP::RoundRobinServiceAssignmentBalance::serviceDown(size_t downedServiceIndex) {
    // migrate all existing clients from the downed service to new services

    // remove the downed service
}
