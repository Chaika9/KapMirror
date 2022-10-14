#include "NetworkComponent.hpp"
#include "KapMirror/NetworkManager.hpp"

using namespace KapMirror;

NetworkComponent::NetworkComponent(std::shared_ptr<KapEngine::GameObject> go, std::string name) : KapEngine::Component(go, name) {
    addRequireComponent("NetworkIdentity");
}

std::shared_ptr<NetworkServer> NetworkComponent::getServer() const {
    return NetworkManager::getInstance()->getServer();
}

std::shared_ptr<NetworkClient> NetworkComponent::getClient() const {
    return NetworkManager::getInstance()->getClient();
}

void NetworkComponent::onAwake() {
    networkIdentity = &getGameObject().getComponent<NetworkIdentity>();
    if (networkIdentity == nullptr) {
        throw std::runtime_error("NetworkComponent: GameObject does not have a NetworkIdentity component");
    }
}

void NetworkComponent::onDestroy() {
    if (isServer()) {
        getServer()->destroyObject(getNetworkId());
    }
}

bool NetworkComponent::isServer() const {
    return networkIdentity->isServer();
}

bool NetworkComponent::isClient() const {
    return networkIdentity->isClient();
}

bool NetworkComponent::isLocal() const {
    return networkIdentity->isLocal();
}
