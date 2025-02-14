#include "udpLb.h"

LoadBalancerUDP::RoundRobinServiceAssignmentBalance::ServiceKeyManager::ServiceKeyManager(const RoundRobinServiceAssignmentBalance& b): balancer(b) {

}

void LoadBalancerUDP::RoundRobinServiceAssignmentBalance::ServiceKeyManager::updateCurrentReadyServiceKey() {
    currentReadyServiceKey = nextReadyServiceKey;
    nextReadyServiceKey = balancer.lb.services.getCircularNextKey(currentReadyServiceKey);
}

LoadBalancerUDP::RoundRobinServiceAssignmentBalance::RoundRobinServiceAssignmentBalance(const LoadBalancerUDP &lb) : BalanceStrategy(lb) {
    skm.setCurrentReadyServiceKeyToLastServiceKey();
}

void LoadBalancerUDP::RoundRobinServiceAssignmentBalance::routePacket(PacketUDP &p) {
    if (clientToService.contains(p.sender)) {
        // packet from the same client must go to the same receiver
        lb.services[clientToService[p.sender]].get()->send(p.data);
    } else {
        // assign the client to the next in line service 
        clientToService[p.sender] = skm.getCurrentReadyServiceKey();
        serviceToClients[skm.getCurrentReadyServiceKey()].push_back(p.sender);
        skm.updateCurrentReadyServiceKey();
    }
}


void LoadBalancerUDP::RoundRobinServiceAssignmentBalance::serviceUp(size_t serviceKey) {
    
}

void LoadBalancerUDP::RoundRobinServiceAssignmentBalance::serviceDown(size_t downedServiceKey) {
    if (lb.services.size() == 0) {
        throw runtime_error("There are 0 services");
    }

    if (downedServiceKey == skm.getCurrentReadyServiceKey()) {
        if (downedServiceKey == skm.getNextReadyServiceKey()) {
            throw runtime_error("Bad next ready service key");
        }

        skm.updateCurrentReadyServiceKey();
    }

    // migrate all existing clients from the downed service to new services
    

    // remove the downed service
}
