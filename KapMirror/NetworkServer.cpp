#include "NetworkServer.hpp"
#include "Runtime/Transport.hpp"
#include "Runtime/Compression.hpp"
#include "KapMirror/Experimental/Components/NetworkIdentity.hpp"
#include "KapEngine.hpp"
#include "Factory.hpp"
#include "Transform.hpp"

#include "UiImage.hpp"
#include "TestNetwork/SpaceShip.hpp"

using namespace KapMirror;

NetworkServer::NetworkServer(NetworkManager& _manager, KapEngine::KapEngine& _engine) : manager(_manager), engine(_engine) {
    initialized = false;
    active = false;
}

void NetworkServer::initialize() {
    if (initialized) {
        return;
    }

    if (Transport::activeTransport == nullptr) {
        KapEngine::Debug::error("NetworkServer: No transport set, cannot initialize");
        throw std::runtime_error("No transport set");
    }

    connections.clear();

    addTransportHandlers();

    initialized = true;
}

void NetworkServer::listen(int maxConnections, int port) {
    initialize();

    this->maxConnections = maxConnections;
    Transport::activeTransport->serverStart(port);

    active = true;
}

void NetworkServer::shutdown() {
    if (initialized) {
        removeTransportHandlers();

        Transport::activeTransport->serverStop();

        initialized = false;
    }

    // Reset all statics here....
    active = false;

    connections.clear();

    // clear events
    onConnectedEvent = nullptr;
    onDisconnectedEvent = nullptr;
}

void NetworkServer::networkEarlyUpdate() {
    if (initialized) {
        Transport::activeTransport->serverEarlyUpdate();
    }
}

void NetworkServer::addTransportHandlers() {
    Transport::activeTransport->onServerConnected = [this](Transport&, int connectionId) {
        onTransportConnect(connectionId);
    };
    Transport::activeTransport->onServerDisconnected = [this](Transport&, int connectionId) {
        onTransportDisconnect(connectionId);
    };
    Transport::activeTransport->onServerDataReceived = [this](Transport&, int connectionId, std::shared_ptr<ArraySegment<byte>> data) {
        onTransportData(connectionId, data);
    };
}

void NetworkServer::removeTransportHandlers() {
    Transport::activeTransport->onServerConnected = nullptr;
    Transport::activeTransport->onServerDisconnected = nullptr;
    Transport::activeTransport->onServerDataReceived = nullptr;
}

void NetworkServer::onTransportConnect(int connectionId) {
    KapEngine::Debug::log("NetworkServer: Client connected: " + std::to_string(connectionId));

    if (connections.containsKey(connectionId)) {
        KapEngine::Debug::warning("NetworkServer: Client already connected: " + std::to_string(connectionId));
        return;
    }

    if (connections.size() > maxConnections) {
        Transport::activeTransport->serverDisconnect(connectionId);
        KapEngine::Debug::warning("NetworkServer: Server full, kicked client " + std::to_string(connectionId));
        return;
    }

    auto connection = std::make_shared<NetworkConnectionToClient>(connectionId);
    addConnection(connection);

    if (onConnectedEvent != nullptr) {
        onConnectedEvent(connection);
    }
}

void NetworkServer::onTransportDisconnect(int connectionId) {
    KapEngine::Debug::log("NetworkServer: Client " + std::to_string(connectionId) + " disconnected");

    std::shared_ptr<NetworkConnectionToClient> connection;
    if (connections.tryGetValue(connectionId, connection)) {
        removeConnection(connectionId);

        if (onDisconnectedEvent != nullptr) {
            onDisconnectedEvent(connection);
        }
    }
}

void NetworkServer::onTransportData(int connectionId, std::shared_ptr<ArraySegment<byte>> data) {
    std::shared_ptr<NetworkConnectionToClient> connection;
    if (connections.tryGetValue(connectionId, connection)) {
        if (!unpackAndInvoke(connection, data)) {
            KapEngine::Debug::warning("NetworkServer: failed to unpack and invoke message. Disconnecting " + std::to_string(connectionId));
            connection->disconnect();
            return;
        }
    }
}

bool NetworkServer::unpackAndInvoke(std::shared_ptr<NetworkConnectionToClient> connection, std::shared_ptr<ArraySegment<byte>> data) {
    if (Compression::activeCompression != nullptr) {
        data = Compression::activeCompression->decompress(data);
    }

    NetworkReader reader(data);

    ushort messageType;
    MessagePacking::unpack(reader, messageType);

    std::shared_ptr<std::function<void(std::shared_ptr<NetworkConnectionToClient>, NetworkReader&)>> handler;
    if (handlers.tryGetValue(messageType, handler)) {
        (*handler)(connection, reader);
        return true;
    }
    return false;
}

bool NetworkServer::addConnection(std::shared_ptr<NetworkConnectionToClient> connection) {
    if (connections.containsKey(connection->getConnectionId())) {
        return false;
    }
    connections[connection->getConnectionId()] = connection;
    return true;
}

bool NetworkServer::removeConnection(int connectionId) {
    return connections.remove(connectionId);
}

// KapEngine

void NetworkServer::spawnObject(std::string prefabName, KapEngine::SceneManagement::Scene &scene, std::shared_ptr<KapEngine::GameObject>& gameObject) {
    KapEngine::Debug::log("NetworkServer: spawnObject");
    engine.getPrefabManager()->instantiatePrefab(prefabName, scene, gameObject);

    if (!gameObject->hasComponent<KapMirror::Experimental::NetworkIdentity>()) {
        KapEngine::Debug::error("NetworkServer: spawnObject: GameObject does not have NetworkIdentity component");
        return;
    }

    manager.__initGameObject(gameObject);

    auto& networkIdentity = gameObject->getComponent<KapMirror::Experimental::NetworkIdentity>();
    networkIdentity.setAuthority(true);
    networkIdentity.onStartServer();

    auto& transform = gameObject->getComponent<KapEngine::Transform>();

    ObjectSpawnMessage message;
    message.networkId = networkIdentity.getNetworkId();
    message.isOwner = !networkIdentity.hasAuthority();
    message.prefabName = prefabName;
    message.x = transform.getLocalPosition().getX();
    message.y = transform.getLocalPosition().getY();
    message.z = transform.getLocalPosition().getZ();
    sendToAll(message);
}

void NetworkServer::destroyObject(unsigned int networkId) {
    KapEngine::Debug::log("NetworkServer: destroyObject");

    std::shared_ptr<KapEngine::GameObject> gameObject;
    if (!findObject(networkId, gameObject)) {
        KapEngine::Debug::error("NetworkServer: destroyObject: GameObject not found");
        return;
    }

    auto& scene = gameObject->getScene();
    scene.destroyGameObject(gameObject);

    ObjectDestroyMessage message;
    message.networkId = networkId;
    sendToAll(message);
}

bool NetworkServer::findObject(unsigned int networkId, std::shared_ptr<KapEngine::GameObject>& gameObject) {
    //TODO: Keep a map of networkId to GameObject
    for (auto& scene : engine.getSceneManager()->getAllScenes()) {
        for (auto& object : scene->getAllObjects()) {
            if (object->hasComponent<KapMirror::Experimental::NetworkIdentity>()) {
                continue;
            }

            auto& networkIdentity = object->getComponent<KapMirror::Experimental::NetworkIdentity>();
            if (networkIdentity.getNetworkId() == networkId) {
                gameObject = object;
                return true;
            }
        }
    }
    return false;
}
