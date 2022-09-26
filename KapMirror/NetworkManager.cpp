#include "NetworkManager.hpp"
#include "Debug.hpp"
#include "KapMirror/Runtime/Transports/Telepathy/TelepathyTransport.hpp"

using namespace KapMirror;

std::shared_ptr<Transport> Transport::activeTransport = nullptr;

NetworkManager::NetworkManager(std::shared_ptr<KapEngine::GameObject> go) : Component(go, "NetworkManager", 2) {
    server = std::make_shared<NetworkServer>();
    client = std::make_shared<NetworkClient>();
}

NetworkManager::~NetworkManager() {
}

void NetworkManager::onAwake() {
    KapEngine::Debug::log("KapMirror | KapEngine | https://github.com/Chaika9/KapMirror");

    if (transport == nullptr) {
        KapEngine::Debug::log("NetworkManager: No transport set, using default transport (TelepathyTransport)");
        setTransport(std::make_shared<KapMirror::TelepathyTransport>());
    }
}

void NetworkManager::onStart() {
    startServer();
}

void NetworkManager::onUpdate() {
}

void NetworkManager::setTransport(std::shared_ptr<Transport> transport) {
    this->transport = transport;
    Transport::activeTransport = transport;
}

void NetworkManager::startServer() {
    KapEngine::Debug::log("NetworkManager: Starting server");
    setupServer();
}

void NetworkManager::setupServer() {
    // always >= 0
    maxConnections = std::max(maxConnections, 0);

    server->listen(maxConnections);
    KapEngine::Debug::log("NetworkManager: Server started listening");
}

void NetworkManager::stopServer() {
}

void NetworkManager::startClient() {
    KapEngine::Debug::log("NetworkManager: Starting client");
    if (client->active()) {
        KapEngine::Debug::warning("NetworkManager: Client already started.");
        return;
    }

    client->connect("127.0.0.1", 7777);
}

void NetworkManager::stopClient() {
}

