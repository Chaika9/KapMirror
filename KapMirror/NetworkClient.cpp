#include "NetworkClient.hpp"
#include "Runtime/Transport.hpp"
#include "Runtime/Compression.hpp"
#include "KapMirror/Experimental/Components/NetworkIdentity.hpp"
#include "KapEngine.hpp"
#include "Factory.hpp"
#include "Transform.hpp"

using namespace KapMirror;

NetworkClient::NetworkClient(KapEngine::KapEngine& _engine) : engine(_engine) {
    connectState = ConnectState::None;
    connection = nullptr;
}

void NetworkClient::connect(std::string ip, int port) {
    if (Transport::activeTransport == nullptr) {
        KapEngine::Debug::error("NetworkClient: No transport set, cannot initialize");
        throw std::runtime_error("No transport set");
    }

    registerSystemHandlers();
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
}

void NetworkClient::networkEarlyUpdate() {
    if (Transport::activeTransport != nullptr) {
        Transport::activeTransport->clientEarlyUpdate();
    }
}

void NetworkClient::addTransportHandlers() {
    Transport::activeTransport->onClientConnected = [this](Transport&) {
        onTransportConnect();
    };
    Transport::activeTransport->onClientDisconnected = [this](Transport&) {
        onTransportDisconnect();
    };
    Transport::activeTransport->onClientDataReceived = [this](Transport&, std::shared_ptr<ArraySegment<byte>> data) {
        onTransportData(data);
    };
}

void NetworkClient::removeTransportHandlers() {
    Transport::activeTransport->onClientConnected = nullptr;
    Transport::activeTransport->onClientDisconnected = nullptr;
    Transport::activeTransport->onClientDataReceived = nullptr;
}

void NetworkClient::registerSystemHandlers() {
    registerHandler<ObjectSpawnMessage>([this](std::shared_ptr<NetworkConnectionToServer> connection, ObjectSpawnMessage& message) {
        onObjectSpawn(message);
    });
    registerHandler<ObjectDestroyMessage>([this](std::shared_ptr<NetworkConnectionToServer> connection, ObjectDestroyMessage& message) {
        onObjectDestroy(message);
    });
    registerHandler<ObjectTransformMessage>([this](std::shared_ptr<NetworkConnectionToServer> connection, ObjectTransformMessage& message) {
        onObjectTransformUpdate(message);
    });
}

void NetworkClient::onTransportConnect() {
    connectState = ConnectState::Connected;

    if (onConnectedEvent != nullptr) {
        onConnectedEvent(connection);
    }
}

void NetworkClient::onTransportDisconnect() {
    if (connectState == ConnectState::Disconnected) {
        return;
    }

    if (onDisconnectedEvent != nullptr) {
        onDisconnectedEvent(connection);
    }

    connectState = ConnectState::Disconnected;

    connection = nullptr;

    removeTransportHandlers();
}

void NetworkClient::onTransportData(std::shared_ptr<ArraySegment<byte>> data) {
    if (!unpackAndInvoke(data)) {
        KapEngine::Debug::warning("NetworkClient: failed to unpack and invoke message. Disconnecting");
        connection->disconnect();
        return;
    }
}

bool NetworkClient::unpackAndInvoke(std::shared_ptr<ArraySegment<byte>> data) {
    if (Compression::activeCompression != nullptr) {
        data = Compression::activeCompression->decompress(data);
    }

    NetworkReader reader(data);

    ushort messageType;
    MessagePacking::unpack(reader, messageType);

    std::shared_ptr<std::function<void(std::shared_ptr<NetworkConnectionToServer>, NetworkReader&)>> handler;
    if (handlers.tryGetValue(messageType, handler)) {
        (*handler)(connection, reader);
        return true;
    }
    return false;
}

// KapEngine

void NetworkClient::onObjectSpawn(ObjectSpawnMessage& message) {
    std::shared_ptr<KapEngine::GameObject> gameObject;
    if (!findObject(message.networkId, gameObject)) {
        auto& scene = engine.getSceneManager()->getScene(message.sceneId);
        if (!engine.getPrefabManager()->instantiatePrefab(message.prefabName, scene, gameObject)) {
            KapEngine::Debug::error("NetworkClient: failed to instantiate prefab " + message.prefabName + " with networkId " + std::to_string(message.networkId));
            return;
        }

        networkObjects[message.networkId] = gameObject;
    }

    if (gameObject == nullptr) {
        KapEngine::Debug::error("NetworkClient: failed to find or instantiate object with network id " + std::to_string(message.networkId));
        return;
    }

    auto& transform = gameObject->getComponent<KapEngine::Transform>();
    transform.setPosition(KapEngine::Tools::Vector3(message.x, message.y, message.z));

    if (!gameObject->hasComponent<KapMirror::Experimental::NetworkIdentity>()) {
        KapEngine::Debug::error("NetworkClient: object " + message.prefabName + " does not have NetworkIdentity component");
        return;
    }

    auto& networkIdentity = gameObject->getComponent<KapMirror::Experimental::NetworkIdentity>();
    networkIdentity.setNetworkId(message.networkId);
    networkIdentity.setAuthority(message.isOwner);
    networkIdentity.onStartClient();
}

void NetworkClient::onObjectDestroy(ObjectDestroyMessage& message) {
    std::shared_ptr<KapEngine::GameObject> gameObject;
    if (findObject(message.networkId, gameObject)) {
        networkObjects.remove(message.networkId);
        gameObject->destroy();
    }
}

void NetworkClient::onObjectTransformUpdate(ObjectTransformMessage& message) {
    std::shared_ptr<KapEngine::GameObject> gameObject;
    if (findObject(message.networkId, gameObject)) {
        auto& transform = gameObject->getComponent<KapEngine::Transform>();
        transform.setPosition(KapEngine::Tools::Vector3(message.x, message.y, message.z));
    }
}

bool NetworkClient::findObject(unsigned int networkId, std::shared_ptr<KapEngine::GameObject>& gameObject) {
    return networkObjects.tryGetValue(networkId, gameObject);
}
