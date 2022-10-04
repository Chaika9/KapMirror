#include "NetworkClient.hpp"
#include "Runtime/Transport.hpp"
#include "Runtime/Compression.hpp"
#include "KapMirror/Experimental/Components/NetworkIdentity.hpp"
#include "KapEngine.hpp"
#include "Factory.hpp"
#include "UiText.hpp"

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
    KapEngine::Debug::log("NetworkClient: onObjectSpawn");
    auto& scene = engine.getSceneManager()->getCurrentScene();

    std::shared_ptr<KapEngine::GameObject> object;
    if (!findObject(message.networkId, scene, object)) {
        // TODO: Get Prefab
        object = KapEngine::Factory::createEmptyGameObject(scene, "SpaceShip");

        auto networkIdentityComponent = std::make_shared<KapMirror::Experimental::NetworkIdentity>(object);
        networkIdentityComponent->setNetworkId(message.networkId);
        object->addComponent(networkIdentityComponent);

        auto textComponent = std::make_shared<KapEngine::UI::Text>(object);
        object->addComponent(textComponent);
        textComponent->setText("SpaceShip:Client");
    }

    auto& shipTransform = object->getComponent<KapEngine::Transform>();
    shipTransform.setPosition(KapEngine::Tools::Vector3(message.x, message.y, message.z));
    shipTransform.setParent(3);

    auto& networkIdentityComponent = object->getComponent<KapMirror::Experimental::NetworkIdentity>();
    networkIdentityComponent.setAuthority(message.isOwner);
}

void NetworkClient::onObjectDestroy(ObjectDestroyMessage& message) {
    KapEngine::Debug::log("NetworkClient: onObjectDestroy");
    auto& scene = engine.getSceneManager()->getCurrentScene();

    std::shared_ptr<KapEngine::GameObject> object;
    if (findObject(message.networkId, scene, object)) {
        scene.destroyGameObject(object);
    }
}

void NetworkClient::onObjectTransformUpdate(ObjectTransformMessage& message) {
    auto& scene = engine.getSceneManager()->getCurrentScene();

    std::shared_ptr<KapEngine::GameObject> object;
    if (findObject(message.networkId, scene, object)) {
        auto& transform = object->getComponent<KapEngine::Transform>();
        transform.setPosition(KapEngine::Tools::Vector3(message.x, message.y, message.z));
    }
}

bool NetworkClient::findObject(unsigned int networkId, KapEngine::SceneManagement::Scene& scene, std::shared_ptr<KapEngine::GameObject>& gameObject) {
    for (auto& go : scene.getAllObjects()) {
        for (auto& component : go->getAllComponents()) {
            auto identity = std::dynamic_pointer_cast<KapMirror::Experimental::NetworkIdentity>(component);
            if (identity) {
                if (identity->getNetworkId() == networkId) {
                    gameObject = go;
                    return true;
                }
            }
        }
    }
    return false;
}
