#include "NetworkServer.hpp"
#include "Runtime/Transport.hpp"
#include "Runtime/Compression.hpp"
#include "NetworkManager.hpp"
#include "Components/NetworkIdentity.hpp"
#include "Components/NetworkComponent.hpp"
#include "KapEngine.hpp"
#include "Factory.hpp"
#include "Transform.hpp"

using namespace KapMirror;

NetworkServer::NetworkServer(NetworkManager& _manager, KapEngine::KEngine& _engine) : manager(_manager), engine(_engine) {
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

    // Send all existing network objects to the client
    for (auto const& [id, gameObject] : networkObjects) {
        sendObject(gameObject, connection);
    }

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

void NetworkServer::spawnObject(std::string prefabName,
    KapEngine::SceneManagement::Scene &scene, KapEngine::Tools::Vector3 position,
    std::function<void(std::shared_ptr<KapEngine::GameObject>&)> playload,
    std::shared_ptr<KapEngine::GameObject>& gameObject) {
    KAP_DEBUG_LOG("NetworkServer: Spawning object " + prefabName);

    if (!engine.getPrefabManager()->instantiatePrefab(prefabName, scene, gameObject)) {
        KapEngine::Debug::error("NetworkServer: Failed to spawn object " + prefabName);
        return;
    }

    if (!gameObject->hasComponent<NetworkIdentity>()) {
        KapEngine::Debug::error("NetworkServer: spawnObject: GameObject does not have NetworkIdentity component");
        gameObject->destroy();
        return;
    }

    auto& networkIdentity = gameObject->getComponent<NetworkIdentity>();

    // Spawn should only be called once per netId
    if (networkObjects.containsKey(networkIdentity.getNetworkId())) {
        KapEngine::Debug::warning("NetworkServer: " + prefabName + " with networkId=" + std::to_string(networkIdentity.getNetworkId()) + " was already spawned.");
        gameObject->destroy();
        return;
    }

    manager.__initGameObject(gameObject);

    networkIdentity.setAuthority(true);
    networkIdentity.onStartServer();

    networkObjects[networkIdentity.getNetworkId()] = gameObject;

    auto& transform = gameObject->getComponent<KapEngine::Transform>();
    transform.setPosition(position);

    if (playload != nullptr) {
        playload(gameObject);
    }

    // @Beta - Custom Payload
    NetworkWriter writer;
    try {
        for (auto& component : gameObject->getAllComponents()) {
            auto comp = std::dynamic_pointer_cast<NetworkComponent>(component);
            if (comp) {
                writer.write(comp->isEnable()); // Is Active
                comp->customPayloadSerialize(writer);
            }
        }
    } catch (...) {
        KapEngine::Debug::error("NetworkServer: Failed to serialize custom payload");
    }

    ObjectSpawnMessage message;
    message.networkId = networkIdentity.getNetworkId();
    message.isOwner = !networkIdentity.hasAuthority();
    message.prefabName = prefabName;
    message.sceneName = scene.getName();
    message.x = position.getX();
    message.y = position.getY();
    message.z = position.getZ();
    message.payload = writer.toArraySegment();
    sendToAll(message);
}

void NetworkServer::destroyObject(unsigned int networkId) {
    KAP_DEBUG_LOG("NetworkServer: Destroying object " + std::to_string(networkId));

    std::shared_ptr<KapEngine::GameObject> gameObject;
    if (!findObject(networkId, gameObject)) {
        KapEngine::Debug::error("NetworkServer: destroyObject: GameObject not found");
        return;
    }

    networkObjects.remove(networkId);
    gameObject->destroy();

    ObjectDestroyMessage message;
    message.networkId = networkId;
    sendToAll(message);
}

void NetworkServer::destroyObject(std::shared_ptr<KapEngine::GameObject> gameObject) {
    if (!gameObject->hasComponent<NetworkIdentity>()) {
        KapEngine::Debug::error("NetworkServer: destroyObject: GameObject does not have NetworkIdentity component");
        return;
    }

    auto& networkIdentity = gameObject->getComponent<NetworkIdentity>();
    destroyObject(networkIdentity.getNetworkId());
}

bool NetworkServer::findObject(unsigned int networkId, std::shared_ptr<KapEngine::GameObject>& gameObject) {
    return networkObjects.tryGetValue(networkId, gameObject);
}

void NetworkServer::sendObject(std::shared_ptr<KapEngine::GameObject> gameObject, std::shared_ptr<NetworkConnectionToClient> connection) {
    if (!gameObject->hasComponent<NetworkIdentity>()) {
        KapEngine::Debug::error("NetworkServer: sendObject: GameObject does not have NetworkIdentity component");
        return;
    }

    auto& networkIdentity = gameObject->getComponent<NetworkIdentity>();
    auto& transform = gameObject->getComponent<KapEngine::Transform>();

    // @Beta - Custom Payload
    NetworkWriter writer;
    try {
        for (auto& component : gameObject->getAllComponents()) {
            auto comp = std::dynamic_pointer_cast<NetworkComponent>(component);
            if (comp) {
                writer.write(comp->isEnable()); // Is Active
                comp->customPayloadSerialize(writer);
            }
        }
    } catch (...) {
        KapEngine::Debug::error("NetworkServer: Failed to serialize custom payload");
    }

    ObjectSpawnMessage message;
    message.networkId = networkIdentity.getNetworkId();
    message.isOwner = !networkIdentity.hasAuthority();
    message.prefabName = gameObject->getPrefabName();
    message.sceneName = gameObject->getScene().getName();
    message.x = transform.getLocalPosition().getX();
    message.y = transform.getLocalPosition().getY();
    message.z = transform.getLocalPosition().getZ();
    message.payload = writer.toArraySegment();
    connection->send(message);
}
