#include "NetworkComponent.hpp"

using namespace KapMirror::Experimental;

NetworkComponent::NetworkComponent(std::shared_ptr<KapEngine::GameObject> go, std::string name) : KapEngine::Component(go, name) {
    addRequireComponent("NetworkIdentity");
}

void NetworkComponent::onAwake() {
    networkIdentity = &getGameObject().getComponent<NetworkIdentity>();
    if (networkIdentity == nullptr) {
        throw std::runtime_error("NetworkComponent: GameObject does not have a NetworkIdentity component");
    }
}

bool NetworkComponent::isServer() const {
    return networkIdentity->isServer();
}

bool NetworkComponent::isClient() const {
    return networkIdentity->isClient();
}
