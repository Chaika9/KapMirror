#include "NetworkManager.hpp"
#include "Debug.hpp"
#include "KapMirror/Runtime/Transports/Telepathy/TelepathyTransport.hpp"

using namespace KapMirror;

std::shared_ptr<Transport> Transport::activeTransport = nullptr;

NetworkManager::NetworkManager(std::shared_ptr<KapEngine::GameObject> go) : Component(go, "NetworkManager") {
    server = std::make_shared<NetworkServer>();
    client = std::make_shared<NetworkClient>();
}

NetworkManager::~NetworkManager() {
    server->shutdown();
    client->disconnect();
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
    server->networkEarlyUpdate();
    client->networkEarlyUpdate();
}

void NetworkManager::setTransport(std::shared_ptr<Transport> transport) {
    this->transport = transport;
    Transport::activeTransport = transport;
}

void NetworkManager::startServer() {
    if (server->isActivated()) {
        KapEngine::Debug::warning("NetworkManager: Server already started");
        return;
    }

    KapEngine::Debug::log("NetworkManager: Starting server");
    setupServer();
}

void NetworkManager::setupServer() {
    // always >= 0
    maxConnections = std::max(maxConnections, 0);

    server->onConnectedEvent = [this](std::shared_ptr<NetworkConnection> connection) {
        onServerClientConnected(connection);
    };
    server->onDisconnectedEvent = [this](std::shared_ptr<NetworkConnection> connection) {
        onServerClientDisconnected(connection);
    };

    server->listen(maxConnections, networkPort);
    KapEngine::Debug::log("NetworkManager: Server started listening");

    onStartServer();
}

void NetworkManager::stopServer() {
    onStopServer();

    server->shutdown();
}

void NetworkManager::startClient() {
    KapEngine::Debug::log("NetworkManager: Starting client");
    if (client->active()) {
        KapEngine::Debug::warning("NetworkManager: Client already started.");
        return;
    }

    client->onConnectedEvent = [this](std::shared_ptr<NetworkConnection> connection) {
        onClientConnected(connection);
    };
    client->onDisconnectedEvent = [this](std::shared_ptr<NetworkConnection> connection) {
        onClientDisconnected(connection);
    };

    client->connect(networkAddress, networkPort);

    onStartClient();
}

void NetworkManager::stopClient() {
    onStopClient();

    client->disconnect();
}

