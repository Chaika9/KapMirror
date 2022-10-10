#include "NetworkManager.hpp"
#include "Debug.hpp"
#include "KapMirror/Runtime/Transports/Telepathy/TelepathyTransport.hpp"

using namespace KapMirror;

std::shared_ptr<Transport> Transport::activeTransport = nullptr;
std::shared_ptr<Compression> Compression::activeCompression = nullptr;

NetworkManager::NetworkManager(std::shared_ptr<KapEngine::GameObject> go) : KapEngine::Component(go, "NetworkManager") {
    server = std::make_shared<NetworkServer>(*this, go->getEngine());
    client = std::make_shared<NetworkClient>(go->getEngine());
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

void NetworkManager::onUpdate() {
    server->networkEarlyUpdate();
    client->networkEarlyUpdate();
}

void NetworkManager::setTransport(std::shared_ptr<Transport> transport) {
    this->transport = transport;
    Transport::activeTransport = transport;
}

void NetworkManager::setCompression(std::shared_ptr<Compression> compression) {
    this->compression = compression;
    Compression::activeCompression = compression;
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

    // TODO: Move this
    auto& scene = getGameObject().getEngine().getSceneManager()->getCurrentScene();

    for (auto& go : scene.getAllObjects()) {
        for (auto& component : go->getAllComponents()) {
            auto comp = std::dynamic_pointer_cast<KapMirror::Experimental::NetworkComponent>(component);
            if (comp) {
                comp->__setServer(server);
            }
        }
    }

    for (auto& go : scene.getAllObjects()) {
        for (auto& component : go->getAllComponents()) {
            auto identity = std::dynamic_pointer_cast<KapMirror::Experimental::NetworkIdentity>(component); //TODO: to hasComponent in KapEngine
            if (identity) {
                identity->onStartServer();
            }
        }
    }
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

    // TODO: Move this
    auto& scene = getGameObject().getEngine().getSceneManager()->getCurrentScene();

    for (auto& go : scene.getAllObjects()) {
        __initGameObject(go);
    }

    for (auto& go : scene.getAllObjects()) {
        for (auto& component : go->getAllComponents()) {
            auto identity = std::dynamic_pointer_cast<KapMirror::Experimental::NetworkIdentity>(component);
            if (identity) {
                identity->onStartClient();
            }
        }
    }
}

void NetworkManager::stopClient() {
    onStopClient();

    client->disconnect();
}

void NetworkManager::__initGameObject(std::shared_ptr<KapEngine::GameObject> go) {
    for (auto& component : go->getAllComponents()) {
        auto comp = std::dynamic_pointer_cast<KapMirror::Experimental::NetworkComponent>(component);
        if (comp) {
            comp->__setServer(server);
            comp->__setClient(client);
        }
    }
}
