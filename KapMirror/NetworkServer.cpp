#include "NetworkServer.hpp"
#include "Core/Transport.hpp"
#include "Core/Compression.hpp"
#include "NetworkManager.hpp"
#include "Components/NetworkIdentity.hpp"
#include "Components/NetworkComponent.hpp"

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
    registerSystemHandlers();

    initialized = true;
}

void NetworkServer::listen(int _maxConnections, int port) {
    initialize();

    maxConnections = _maxConnections;
    Transport::activeTransport->serverStart(port);

    active = true;
}

void NetworkServer::shutdown() {
    if (initialized) {
        disconnectAll();

        removeTransportHandlers();

        Transport::activeTransport->serverStop();

        initialized = false;
    }

    // Reset all statics here....
    active = false;

    connections.clear();
}

void NetworkServer::networkEarlyUpdate() const {
    if (initialized) {
        Transport::activeTransport->serverEarlyUpdate();
    }
}

void NetworkServer::addTransportHandlers() {
    Transport::activeTransport->onServerConnected += [this](Transport&, int connectionId) { onTransportConnect(connectionId); };
    Transport::activeTransport->onServerDisconnected += [this](Transport&, int connectionId) { onTransportDisconnect(connectionId); };
    Transport::activeTransport->onServerDataReceived +=
        [this](Transport&, int connectionId, const std::shared_ptr<ArraySegment<byte>>& data) { onTransportData(connectionId, data); };
}

void NetworkServer::removeTransportHandlers() {
    // clear all events
    Transport::activeTransport->onServerConnected.clear();
    Transport::activeTransport->onServerDisconnected.clear();
    Transport::activeTransport->onServerDataReceived.clear();
}

void NetworkServer::registerSystemHandlers() {
    registerHandler<ObjectSpawnMessage>(
        [this](const std::shared_ptr<NetworkConnectionToClient>& conn, ObjectSpawnMessage& message) { onObjectSpawn(message); });
    registerHandler<ObjectTransformMessage>([this](const std::shared_ptr<NetworkConnectionToClient>& conn,
                                                   ObjectTransformMessage& message) { onObjectTransformUpdate(message); });
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

    onConnectedEvent(connection);
}

void NetworkServer::onTransportDisconnect(int connectionId) {
    KapEngine::Debug::log("NetworkServer: Client " + std::to_string(connectionId) + " disconnected");

    std::shared_ptr<NetworkConnectionToClient> connection;
    if (connections.tryGetValue(connectionId, connection)) {
        removeConnection(connectionId);
        onDisconnectedEvent(connection);
    }
}

void NetworkServer::onTransportData(int connectionId, const std::shared_ptr<ArraySegment<byte>>& data) {
    std::shared_ptr<NetworkConnectionToClient> connection;
    if (connections.tryGetValue(connectionId, connection)) {
        if (!unpackAndInvoke(connection, data)) {
            KapEngine::Debug::warning("NetworkServer: failed to unpack and invoke message. Disconnecting " + std::to_string(connectionId));
            connection->disconnect();
            return;
        }
    }
}

bool NetworkServer::unpackAndInvoke(const std::shared_ptr<NetworkConnectionToClient>& connection,
                                    const std::shared_ptr<ArraySegment<byte>>& data) {
    std::shared_ptr<ArraySegment<byte>> dataToRead = data;
    if (Compression::activeCompression != nullptr) {
        dataToRead = Compression::activeCompression->decompress(dataToRead);
    }

    NetworkReader reader(dataToRead);

    ushort messageType;
    MessagePacking::unpack(reader, messageType);

    std::shared_ptr<std::function<void(std::shared_ptr<NetworkConnectionToClient>, NetworkReader&)>> handler;
    if (handlers.tryGetValue(messageType, handler)) {
        (*handler)(connection, reader);
        return true;
    }
    return false;
}

bool NetworkServer::addConnection(const std::shared_ptr<NetworkConnectionToClient>& connection) {
    if (connections.containsKey(connection->getConnectionId())) {
        return false;
    }
    connections[connection->getConnectionId()] = connection;
    return true;
}

bool NetworkServer::removeConnection(int connectionId) { return connections.remove(connectionId); }

#pragma region KapEngine

void NetworkServer::onObjectSpawn(ObjectSpawnMessage& message) {
    std::shared_ptr<KapEngine::GameObject> gameObject;
    if (!getExistingObject(message.networkId, gameObject)) {
        return;
    }

    if (gameObject == nullptr) {
        KapEngine::Debug::error("NetworkServer: failed to find or instantiate object with network id " + std::to_string(message.networkId));
        return;
    }

    if (!gameObject->hasComponent<NetworkIdentity>()) {
        KapEngine::Debug::error("NetworkServer: object " + message.prefabName + " does not have NetworkIdentity component");
        return;
    }

    auto& identity = gameObject->getComponent<NetworkIdentity>();
    if (!identity.hasAuthority()) {
        KapEngine::Debug::warning("NetworkServer: object " + std::to_string(identity.getNetworkId()) + " does not have authority");
        return;
    }

    auto& transform = gameObject->getComponent<KapEngine::Transform>();
    transform.setPosition(message.position);
    transform.setRotation(message.rotation);
    transform.setScale(message.scale);

    // Deserialize all components
    NetworkReader reader(message.payload);
    try {
        for (auto& component : gameObject->getAllComponents()) {
            auto networkCompenent = std::dynamic_pointer_cast<NetworkComponent>(component);
            if (networkCompenent) {
                networkCompenent->setActive(reader.read<bool>()); // isActive
                networkCompenent->deserialize(reader);
            }
        }
    } catch (...) { KapEngine::Debug::error("NetworkServer: failed to deserialize custom payload for object " + message.prefabName); }

    for (auto& component : gameObject->getAllComponents()) {
        auto networkCompenent = std::dynamic_pointer_cast<NetworkComponent>(component);
        if (networkCompenent) {
            try {
                networkCompenent->onObjectUpdate();
            } catch (std::exception& e) { KapEngine::Debug::error("NetworkServer: Exception in onObjectUpdate: " + std::string(e.what())); }
        }
    }

    ObjectSpawnMessage objectSpawnMessage;
    objectSpawnMessage.networkId = identity.getNetworkId();
    objectSpawnMessage.isOwner = identity.hasAuthority();
    objectSpawnMessage.prefabName = gameObject->getPrefabName();
    objectSpawnMessage.sceneName = gameObject->getScene().getName();
    objectSpawnMessage.position = transform.getLocalPosition();
    objectSpawnMessage.rotation = transform.getLocalRotation();
    objectSpawnMessage.scale = transform.getLocalScale();
    objectSpawnMessage.payload = message.payload;
    sendToAll(objectSpawnMessage);
}

void NetworkServer::spawnObject(const std::string& prefabName, KapEngine::SceneManagement::Scene& scene,
                                const KapEngine::Tools::Vector3& position,
                                const std::function<void(const std::shared_ptr<KapEngine::GameObject>&)>& playload,
                                std::shared_ptr<KapEngine::GameObject>& gameObject) {

    if (!engine.getPrefabManager()->instantiatePrefab(prefabName, scene, gameObject)) {
        KapEngine::Debug::error("NetworkServer: Failed to spawn object " + prefabName);
        return;
    }

    if (!gameObject->hasComponent<NetworkIdentity>()) {
        KapEngine::Debug::error("NetworkServer: spawnObject: GameObject does not have NetworkIdentity component");
        gameObject->destroy();
        return;
    }

    auto& identity = gameObject->getComponent<NetworkIdentity>();

    // Spawn should only be called once per netId
    if (networkObjects.containsKey(identity.getNetworkId())) {
        KapEngine::Debug::warning("NetworkServer: " + prefabName + " with networkId=" + std::to_string(identity.getNetworkId()) +
                                  " was already spawned.");
        gameObject->destroy();
        return;
    }

    for (auto& component : gameObject->getAllComponents()) {
        auto networkCompenent = std::dynamic_pointer_cast<NetworkComponent>(component);
        if (networkCompenent) {
            networkCompenent->_setNetworkIdentity(&identity);
        }
    }

    identity.setAuthority(false);

    try {
        identity.onStartServer();
    } catch (std::exception& e) {
        KapEngine::Debug::error("NetworkServer: Exception in onStartServer: " + std::string(e.what()));
        return;
    }

    networkObjects[identity.getNetworkId()] = gameObject;

    auto& transform = gameObject->getComponent<KapEngine::Transform>();
    transform.setPosition(position);

    if (playload != nullptr) {
        playload(gameObject);
    }

    for (auto& component : gameObject->getAllComponents()) {
        auto networkCompenent = std::dynamic_pointer_cast<NetworkComponent>(component);
        if (networkCompenent) {
            try {
                networkCompenent->onObjectUpdate();
            } catch (std::exception& e) { KapEngine::Debug::error("NetworkServer: Exception in onObjectUpdate: " + std::string(e.what())); }
        }
    }

    // Serialize all components
    NetworkWriter writer;
    try {
        for (auto& component : gameObject->getAllComponents()) {
            auto networkCompenent = std::dynamic_pointer_cast<NetworkComponent>(component);
            if (networkCompenent) {
                writer.write(networkCompenent->isEnable()); // isActive
                networkCompenent->serialize(writer);
            }
        }
    } catch (...) { KapEngine::Debug::error("NetworkServer: Failed to serialize custom payload"); }

    ObjectSpawnMessage message;
    message.networkId = identity.getNetworkId();
    message.isOwner = identity.hasAuthority();
    message.prefabName = prefabName;
    message.sceneName = scene.getName();
    message.position = transform.getLocalPosition();
    message.rotation = transform.getLocalRotation();
    message.scale = transform.getLocalScale();
    message.payload = writer.toArraySegment();
    sendToAll(message);
}

void NetworkServer::unSpawn(const std::shared_ptr<KapEngine::GameObject>& gameObject) {
    if (gameObject == nullptr) {
        KapEngine::Debug::error("NetworkServer: Cannot unSpawn, gameObject is null");
        return;
    }

    if (!gameObject->hasComponent<NetworkIdentity>()) {
        return;
    }

    auto& identity = gameObject->getComponent<NetworkIdentity>();
    destroyObject(identity.getNetworkId());
}

void NetworkServer::destroyObject(unsigned int networkId) {
    std::shared_ptr<KapEngine::GameObject> gameObject;
    if (!getExistingObject(networkId, gameObject)) {
        return;
    }

    networkObjects.remove(networkId);

    if (!gameObject->hasComponent<NetworkIdentity>()) {
        return;
    }

    auto& identity = gameObject->getComponent<NetworkIdentity>();

    try {
        identity.onStopServer();
    } catch (std::exception& e) { KapEngine::Debug::error("NetworkServer: Exception in onStopServer: " + std::string(e.what())); }

    gameObject->destroy();

    ObjectDestroyMessage message;
    message.networkId = networkId;
    sendToAll(message);
}

void NetworkServer::updateObject(unsigned int id) {
    std::shared_ptr<KapEngine::GameObject> gameObject;
    if (!getExistingObject(id, gameObject)) {
        return;
    }

    if (!gameObject->hasComponent<NetworkIdentity>()) {
        KapEngine::Debug::error("NetworkServer: object " + gameObject->getPrefabName() + " does not have NetworkIdentity component");
        return;
    }

    auto& identity = gameObject->getComponent<NetworkIdentity>();
    auto& transform = gameObject->getComponent<KapEngine::Transform>();

    NetworkWriter writer;
    try {
        for (auto& component : gameObject->getAllComponents()) {
            auto networkCompenent = std::dynamic_pointer_cast<NetworkComponent>(component);
            if (networkCompenent) {
                writer.write(networkCompenent->isEnable()); // isActive
                networkCompenent->serialize(writer);
            }
        }
    } catch (...) { KapEngine::Debug::error("NetworkServer: Failed to serialize custom payload"); }

    ObjectSpawnMessage message;
    message.networkId = identity.getNetworkId();
    message.isOwner = identity.hasAuthority();
    message.prefabName = gameObject->getPrefabName();
    message.sceneName = gameObject->getScene().getName();
    message.position = transform.getLocalPosition();
    message.rotation = transform.getLocalRotation();
    message.scale = transform.getLocalScale();
    message.payload = writer.toArraySegment();
    sendToAll(message);
}

void NetworkServer::sendObject(const std::shared_ptr<KapEngine::GameObject>& gameObject,
                               const std::shared_ptr<NetworkConnectionToClient>& connection) {
    if (!gameObject->hasComponent<NetworkIdentity>()) {
        KapEngine::Debug::error("NetworkServer: sendObject: GameObject does not have NetworkIdentity component");
        return;
    }

    auto& identity = gameObject->getComponent<NetworkIdentity>();
    auto& transform = gameObject->getComponent<KapEngine::Transform>();

    // Serialize all components
    NetworkWriter writer;
    try {
        for (auto& component : gameObject->getAllComponents()) {
            auto networkCompenent = std::dynamic_pointer_cast<NetworkComponent>(component);
            if (networkCompenent) {
                writer.write(networkCompenent->isEnable()); // isActive
                networkCompenent->serialize(writer);
            }
        }
    } catch (...) { KapEngine::Debug::error("NetworkServer: Failed to serialize custom payload"); }

    ObjectSpawnMessage message;
    message.networkId = identity.getNetworkId();
    message.isOwner = identity.hasAuthority();
    message.prefabName = gameObject->getPrefabName();
    message.sceneName = gameObject->getScene().getName();
    message.position = transform.getLocalPosition();
    message.rotation = transform.getLocalRotation();
    message.scale = transform.getLocalScale();
    message.payload = writer.toArraySegment();
    connection->send(message);
}

void NetworkServer::onObjectTransformUpdate(ObjectTransformMessage& message) {
    std::shared_ptr<KapEngine::GameObject> gameObject;
    if (!getExistingObject(message.networkId, gameObject)) {
        return;
    }

    auto& identity = gameObject->getComponent<NetworkIdentity>();
    if (identity.hasAuthority()) {
        return;
    }

    auto& transform = gameObject->getComponent<KapEngine::Transform>();
    transform.setPosition(message.position);
    transform.setRotation(message.rotation);
    transform.setScale(message.scale);
}

#pragma endregion
