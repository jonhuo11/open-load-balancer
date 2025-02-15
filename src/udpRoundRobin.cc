#include "udpLb.h"

LoadBalancerUDP::RoundRobinBalance::ServiceKeyManager::ServiceKeyManager(const RoundRobinBalance& b): balancer(b) {

}

void LoadBalancerUDP::RoundRobinBalance::ServiceKeyManager::updateCurrentReadyServiceKey() {
    currentReadyServiceKey = nextReadyServiceKey;
    nextReadyServiceKey = balancer.lb.services.getCircularNextKey(currentReadyServiceKey);
}

void LoadBalancerUDP::RoundRobinBalance::ServiceKeyManager::updateNextReadyServiceKey() {
    nextReadyServiceKey = balancer.lb.services.getCircularNextKey(currentReadyServiceKey);
}

void LoadBalancerUDP::RoundRobinBalance::ServiceKeyManager::setCurrentReadyServiceKeyToLastServiceKey() {
    currentReadyServiceKey = balancer.lb.services.lastKey();
    updateNextReadyServiceKey();
}

LoadBalancerUDP::RoundRobinBalance::RoundRobinBalance(const LoadBalancerUDP &lb) 
    : BalanceStrategy(lb), skm(*this) {
    skm.setCurrentReadyServiceKeyToLastServiceKey();
}

size_t LoadBalancerUDP::RoundRobinBalance::RoundRobinBalance::ServiceKeyManager::getCurrentReadyServiceKey() const {
    return currentReadyServiceKey;
}

size_t LoadBalancerUDP::RoundRobinBalance::RoundRobinBalance::ServiceKeyManager::getNextReadyServiceKey() const {
    return nextReadyServiceKey;
}

void LoadBalancerUDP::RoundRobinBalance::routePacket(const PacketUDP &p) {
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


void LoadBalancerUDP::RoundRobinBalance::serviceUp(size_t serviceKey) {
    
}

void LoadBalancerUDP::RoundRobinBalance::serviceDown(size_t downedServiceKey) {
    if (lb.services.size() == 0) {
        throw runtime_error("There are 0 services");
    }
    if (downedServiceKey == skm.getCurrentReadyServiceKey() && downedServiceKey == skm.getNextReadyServiceKey()) {
        throw runtime_error("Bad service key states");
    }

    if (downedServiceKey == skm.getCurrentReadyServiceKey()) {
        skm.updateCurrentReadyServiceKey();
    } else if (downedServiceKey == skm.getNextReadyServiceKey()) {
        skm.updateNextReadyServiceKey();
    }

    // migrate all existing clients from the downed service to new services
    // assume the skm keys are valid
    for (auto& clientId : serviceToClients[downedServiceKey]) {
        clientToService[clientId] = skm.getCurrentReadyServiceKey();
    }
    auto& downedServiceClientList = serviceToClients[downedServiceKey];
    auto& destinationServiceClientList = serviceToClients[skm.getCurrentReadyServiceKey()];
    destinationServiceClientList.insert(destinationServiceClientList.end(), downedServiceClientList.begin(), downedServiceClientList.end());

    // remove the downed service
    serviceToClients.erase(downedServiceKey);
    
}
