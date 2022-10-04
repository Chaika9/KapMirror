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

void NetworkServer::spawnObject() {
    KapEngine::Debug::log("NetworkServer: spawnObject");
    auto& scene = engine.getSceneManager()->getCurrentScene();

    auto object = KapEngine::Factory::createEmptyGameObject(scene, "SpaceShip");

    auto networkIdentityComponent = std::make_shared<KapMirror::Experimental::NetworkIdentity>(object);
    object->addComponent(networkIdentityComponent);

    auto networkTransformComponent = std::make_shared<KapMirror::Experimental::NetworkTransform>(object);
    networkTransformComponent->setClientAuthority(false);
    networkTransformComponent->setSendRate(30);
    object->addComponent(networkTransformComponent);

    auto imageComponent = std::make_shared<KapEngine::UI::Image>(object);
    object->addComponent(imageComponent);
    imageComponent->setPathSprite("Assets/Textures/SpaceShip.png");
    imageComponent->setRectangle({0, 0, 99, 75});

    auto shipComponent = std::make_shared<RType::Component::SpaceShip>(object);
    object->addComponent(shipComponent);

    auto& shipTransform = object->getComponent<KapEngine::Transform>();
    shipTransform.setPosition(KapEngine::Tools::Vector3(10.f, 200.f, 0.f));
    shipTransform.setScale(KapEngine::Tools::Vector3(50.f, 50.f, 0.f));
    shipTransform.setParent(3);

    manager.__initGameObject(object);

    networkIdentityComponent->setAuthority(true);
    networkIdentityComponent->onStartServer();

    ObjectSpawnMessage message;
    message.networkId = networkIdentityComponent->getNetworkId();
    message.isOwner = !networkIdentityComponent->hasAuthority();
    message.x = shipTransform.getLocalPosition().getX();
    message.y = shipTransform.getLocalPosition().getY();
    message.z = shipTransform.getLocalPosition().getZ();

    KapEngine::Debug::log("NetworkServer: spawnObject:networkId: " + std::to_string(message.networkId));
    KapEngine::Debug::log("NetworkServer: spawnObject:isOwner: " + std::to_string(message.isOwner));
    KapEngine::Debug::log("NetworkServer: spawnObject:x: " + std::to_string(message.x));
    KapEngine::Debug::log("NetworkServer: spawnObject:y: " + std::to_string(message.y));
    KapEngine::Debug::log("NetworkServer: spawnObject:z: " + std::to_string(message.z));
    sendToAll(message);
}
