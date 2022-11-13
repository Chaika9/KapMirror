#include "NetworkClient.hpp"
#include "Core/Transport.hpp"
#include "Core/Compression.hpp"
#include "NetworkManager.hpp"
#include "Components/NetworkIdentity.hpp"
#include "Components/NetworkComponent.hpp"
#include "KapEngine.hpp"
#include "Transform.hpp"

using namespace KapMirror;

NetworkClient::NetworkClient(NetworkManager& _manager, KapEngine::KEngine& _engine) : manager(_manager), engine(_engine) {
    connectState = ConnectState::None;
    connection = nullptr;
}

void NetworkClient::connect(const std::string& ip, int port) {
    if (Transport::activeTransport == nullptr) {
        KapEngine::Debug::error("NetworkClient: No transport set, cannot initialize");
        throw std::runtime_error("No transport set");
    }

    registerSystemHandlers();

    removeTransportHandlers();
    addTransportHandlers();

    connectState = ConnectState::Connecting;
    Transport::activeTransport->clientConnect(ip, port);

    connection = std::make_shared<NetworkConnectionToServer>();
}

void NetworkClient::disconnect() {
    if (connectState != ConnectState::Connecting && connectState != ConnectState::Connected) {
        return;
    }

    connectState = ConnectState::Disconnecting;

    Transport::activeTransport->clientDisconnect();

    // Clear all network objects
    networkObjects.clear();
}

void NetworkClient::networkEarlyUpdate() {
    if (Transport::activeTransport != nullptr) {
        Transport::activeTransport->clientEarlyUpdate();
    }
}

void NetworkClient::addTransportHandlers() {
    Transport::activeTransport->onClientConnected += [this](Transport&) { onTransportConnect(); };
    Transport::activeTransport->onClientDisconnected += [this](Transport&) { onTransportDisconnect(); };
    Transport::activeTransport->onClientDataReceived +=
        [this](Transport&, const std::shared_ptr<ArraySegment<byte>>& data) { onTransportData(data); };
}

void NetworkClient::removeTransportHandlers() {
    // clear all events
    Transport::activeTransport->onClientConnected.clear();
    Transport::activeTransport->onClientDisconnected.clear();
    Transport::activeTransport->onClientDataReceived.clear();
}

void NetworkClient::registerSystemHandlers() {
    registerHandler<ObjectSpawnMessage>(
        [this](const std::shared_ptr<NetworkConnectionToServer>& conn, ObjectSpawnMessage& message) { onObjectSpawn(message); });
    registerHandler<ObjectDestroyMessage>(
        [this](const std::shared_ptr<NetworkConnectionToServer>& conn, ObjectDestroyMessage& message) { onObjectDestroy(message); });
    registerHandler<ObjectTransformMessage>([this](const std::shared_ptr<NetworkConnectionToServer>& conn,
                                                   ObjectTransformMessage& message) { onObjectTransformUpdate(message); });
}

void NetworkClient::onTransportConnect() {
    connectState = ConnectState::Connected;
    onConnectedEvent(connection);
}

void NetworkClient::onTransportDisconnect() {
    if (connectState == ConnectState::Disconnected) {
        return;
    }

    onDisconnectedEvent(connection);

    connectState = ConnectState::Disconnected;

    connection = nullptr;
}

void NetworkClient::onTransportData(const std::shared_ptr<ArraySegment<byte>>& data) {
    if (!unpackAndInvoke(data)) {
        KapEngine::Debug::warning("NetworkClient: failed to unpack and invoke message. Disconnecting");
        connection->disconnect();
        return;
    }
}

bool NetworkClient::unpackAndInvoke(const std::shared_ptr<ArraySegment<byte>>& data) {
    std::shared_ptr<ArraySegment<byte>> dataToRead = data;
    if (Compression::activeCompression != nullptr) {
        dataToRead = Compression::activeCompression->decompress(dataToRead);
    }

    NetworkReader reader(dataToRead);

    ushort messageType;
    MessagePacking::unpack(reader, messageType);

    std::shared_ptr<std::function<void(std::shared_ptr<NetworkConnectionToServer>, NetworkReader&)>> handler;
    if (handlers.tryGetValue(messageType, handler)) {
        (*handler)(connection, reader);
        return true;
    }
    return false;
}

#pragma region KapEngine

void NetworkClient::onObjectSpawn(ObjectSpawnMessage& message) {
    bool isNew = false;
    std::shared_ptr<KapEngine::GameObject> gameObject;
    if (!getExistingObject(message.networkId, gameObject)) {
        auto& scene = engine.getSceneManager()->getScene(message.sceneName);
        if (!engine.getPrefabManager()->instantiatePrefab(message.prefabName, scene, gameObject)) {
            KapEngine::Debug::error("NetworkClient: failed to instantiate prefab " + message.prefabName + " with networkId " +
                                    std::to_string(message.networkId));
            return;
        }

        isNew = true;
        networkObjects[message.networkId] = gameObject;
    }

    if (gameObject == nullptr) {
        KapEngine::Debug::error("NetworkClient: failed to find or instantiate object with network id " + std::to_string(message.networkId));
        return;
    }

    if (!gameObject->hasComponent<NetworkIdentity>()) {
        KapEngine::Debug::error("NetworkClient: object " + message.prefabName + " does not have NetworkIdentity component");
        return;
    }

    auto& transform = gameObject->getComponent<KapEngine::Transform>();
    transform.setPosition(message.position);
    transform.setRotation(message.rotation);
    transform.setScale(message.scale);

    auto& identity = gameObject->getComponent<NetworkIdentity>();

    for (auto& component : gameObject->getAllComponents()) {
        auto networkCompenent = std::dynamic_pointer_cast<NetworkComponent>(component);
        if (networkCompenent) {
            networkCompenent->_setNetworkIdentity(&identity);
        }
    }

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
    } catch (...) { KapEngine::Debug::error("NetworkClient: failed to deserialize custom payload for object " + message.prefabName); }

    if (isNew) {
        identity.setAuthority(message.isOwner);
        identity.setNetworkId(message.networkId);

        try {
            identity.onStartClient();
        } catch (std::exception& e) { KapEngine::Debug::error("NetworkClient: Exception in onStartClient: " + std::string(e.what())); }
    }

    for (auto& component : gameObject->getAllComponents()) {
        auto networkCompenent = std::dynamic_pointer_cast<NetworkComponent>(component);
        if (networkCompenent) {
            try {
                networkCompenent->onObjectUpdate();
            } catch (std::exception& e) { KapEngine::Debug::error("NetworkClient: Exception in onObjectUpdate: " + std::string(e.what())); }
        }
    }
}

void NetworkClient::onObjectDestroy(ObjectDestroyMessage& message) {
    std::shared_ptr<KapEngine::GameObject> gameObject;
    if (!getExistingObject(message.networkId, gameObject)) {
        return;
    }

    networkObjects.remove(message.networkId);

    if (!gameObject->hasComponent<NetworkIdentity>()) {
        return;
    }

    auto& identity = gameObject->getComponent<NetworkIdentity>();

    try {
        identity.onStopClient();
    } catch (std::exception& e) { KapEngine::Debug::error("NetworkClient: Exception in onStopClient: " + std::string(e.what())); }

    gameObject->destroy();
}

void NetworkClient::onObjectTransformUpdate(ObjectTransformMessage& message) {
    std::shared_ptr<KapEngine::GameObject> gameObject;
    if (!getExistingObject(message.networkId, gameObject)) {
        return;
    }

    auto& transform = gameObject->getComponent<KapEngine::Transform>();
    transform.setPosition(message.position);
    transform.setRotation(message.rotation);
    transform.setScale(message.scale);
}

void NetworkClient::updateObject(unsigned int id) {
    std::shared_ptr<KapEngine::GameObject> gameObject;
    if (!getExistingObject(id, gameObject)) {
        KapEngine::Debug::warning("NetworkClient: updateObject: GameObject not found");
        return;
    }

    if (!gameObject->hasComponent<NetworkIdentity>()) {
        KapEngine::Debug::error("NetworkClient: object " + gameObject->getPrefabName() + " does not have NetworkIdentity component");
        return;
    }

    NetworkWriter writer;
    try {
        for (auto& component : gameObject->getAllComponents()) {
            auto networkCompenent = std::dynamic_pointer_cast<NetworkComponent>(component);
            if (networkCompenent) {
                writer.write(networkCompenent->isEnable()); // isActive
                networkCompenent->serialize(writer);
            }
        }
    } catch (...) { KapEngine::Debug::error("NetworkClient: Failed to serialize custom payload"); }

    auto& identity = gameObject->getComponent<NetworkIdentity>();
    auto& transform = gameObject->getComponent<KapEngine::Transform>();

    ObjectSpawnMessage message;
    message.networkId = identity.getNetworkId();
    message.isOwner = identity.hasAuthority();
    message.prefabName = gameObject->getPrefabName();
    message.sceneName = gameObject->getScene().getName();
    message.position = transform.getLocalPosition();
    message.rotation = transform.getLocalRotation();
    message.scale = transform.getLocalScale();
    message.payload = writer.toArraySegment();
    send(message);
}

#pragma endregion
