#include "NetworkIdentity.hpp"
#include "Debug.hpp"
#include "NetworkComponent.hpp"

using namespace KapMirror;

int NetworkIdentity::nextNetworkId = 1;

NetworkIdentity::NetworkIdentity(std::shared_ptr<KapEngine::GameObject> go) : KapEngine::Component(go, "NetworkIdentity") {
    _networkId = 0;
    _hasAuthority = false;
    _isServer = false;
    _isClient = false;
}

void NetworkIdentity::onStartServer() {
    // do nothing if already spawned
    if (_isServer) {
        return;
    }

    // set isServer flag
    _isServer = true;

    if (_networkId != 0) {
        return;
    }

    _networkId = nextNetworkId++;

    for (auto& component : getGameObject().getAllComponents()) {
        auto networkComponent = std::dynamic_pointer_cast<NetworkComponent>(component);
        if (networkComponent != nullptr) {
            try {
                networkComponent->onStartServer();
            } catch (std::exception& e) {
                KapEngine::Debug::error("NetworkIdentity: Exception in onStartServer: " + std::string(e.what()));
            }
        }
    }
}

void NetworkIdentity::onStopServer() {
    for (auto& component : getGameObject().getAllComponents()) {
        auto networkComponent = std::dynamic_pointer_cast<NetworkComponent>(component);
        if (networkComponent != nullptr) {
            try {
                networkComponent->onStopServer();
            } catch (std::exception& e) { KapEngine::Debug::error("NetworkIdentity: Exception in onStopServer: " + std::string(e.what())); }
        }
    }
}

void NetworkIdentity::onStartClient() {
    // do nothing if already spawned
    if (_isClient) {
        return;
    }

    // set isClient flag
    _isClient = true;

    for (auto& component : getGameObject().getAllComponents()) {
        auto networkComponent = std::dynamic_pointer_cast<NetworkComponent>(component);
        if (networkComponent != nullptr) {
            try {
                networkComponent->onStartClient();
            } catch (std::exception& e) {
                KapEngine::Debug::error("NetworkIdentity: Exception in onStartClient: " + std::string(e.what()));
            }
        }
    }
}

void NetworkIdentity::onStopClient() {
    for (auto& component : getGameObject().getAllComponents()) {
        auto networkComponent = std::dynamic_pointer_cast<NetworkComponent>(component);
        if (networkComponent != nullptr) {
            try {
                networkComponent->onStopClient();
            } catch (std::exception& e) { KapEngine::Debug::error("NetworkIdentity: Exception in onStopClient: " + std::string(e.what())); }
        }
    }
}
