#include "NetworkManager.hpp"
#include "Debug.hpp"
#include "Runtime/Transports/Telepathy/TelepathyTransport.hpp"

using namespace KapMirror;

NetworkManager* NetworkManager::instance = nullptr;

std::shared_ptr<Transport> Transport::activeTransport = nullptr;
std::shared_ptr<Compression> Compression::activeCompression = nullptr;

NetworkManager::NetworkManager(std::shared_ptr<KapEngine::GameObject> go) : KapEngine::Component(go, "NetworkManager") {
    server = std::make_shared<NetworkServer>(*this, go->getEngine());
    client = std::make_shared<NetworkClient>(*this, go->getEngine());
}

NetworkManager::~NetworkManager() {
    server->shutdown();
    client->disconnect();
}

void NetworkManager::initializeSingleton() {
    if (instance != nullptr && instance == this) {
        return;
    }

    instance = this;
}

void NetworkManager::onAwake() {
    initializeSingleton();
    KapEngine::Debug::log("KapMirror | KapEngine | https://github.com/Chaika9/KapMirror");

    if (transport == nullptr) {
        KapEngine::Debug::log("NetworkManager: No transport set, using default transport (TelepathyTransport)");
        setTransport(std::make_shared<TelepathyTransport>());
    }
}

void NetworkManager::onFixedUpdate() {
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
    KapEngine::Debug::log("NetworkManager: Server started listening on port " + std::to_string(networkPort));

    onStartServer();

    // TODO: Move this
    auto& scene = getGameObject().getEngine().getSceneManager()->getCurrentScene();
    for (auto& go : scene.getAllObjects()) {
        for (auto& component : go->getAllComponents()) {
            auto identity = std::dynamic_pointer_cast<NetworkIdentity>(component); //TODO: to hasComponent in KapEngine
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
    if (client->isActive()) {
        KapEngine::Debug::warning("NetworkManager: Client already started.");
        return;
    }

    client->onConnectedEvent = [this](std::shared_ptr<NetworkConnection> connection) {
        onClientConnected(connection);
    };
    client->onDisconnectedEvent = [this](std::shared_ptr<NetworkConnection> connection) {
        onClientDisconnected(connection);
    };

    KapEngine::Debug::log("NetworkManager: Connecting to server \"" + networkAddress + ":" + std::to_string(networkPort) + "\"");
    client->connect(networkAddress, networkPort);

    onStartClient();

    // TODO: Move this
    auto& scene = getGameObject().getEngine().getSceneManager()->getCurrentScene();
    for (auto& go : scene.getAllObjects()) {
        for (auto& component : go->getAllComponents()) {
            auto identity = std::dynamic_pointer_cast<NetworkIdentity>(component);
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
