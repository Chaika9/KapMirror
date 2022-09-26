#include "NetworkClient.hpp"
#include "Runtime/Transport.hpp"
#include "KapEngine.hpp"
#include "Debug.hpp"

using namespace KapMirror;

NetworkClient::NetworkClient() {
    connectState = ConnectState::None;
}

void NetworkClient::connect(std::string ip, int port) {
    if (Transport::activeTransport == nullptr) {
        KapEngine::Debug::error("NetworkClient: No transport set, cannot initialize");
        throw std::runtime_error("No transport set");
    }

    addTransportHandlers();

    connectState = ConnectState::Connecting;
    Transport::activeTransport->clientConnect(ip, port);
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
    Transport::activeTransport->onClientConnected = [this](Transport& t) {
        std::cout << "NetworkClient: Connected" << std::endl;
        onTransportConnect();
    };
    Transport::activeTransport->onClientDisconnected = [this](Transport& t) {
        std::cout << "NetworkClient: Disconnected" << std::endl;
        onTransportDisconnect();
    };
    Transport::activeTransport->onClientDataReceived = [this](Transport& t, std::shared_ptr<ArraySegment<byte>> data) {
        onTransportData(data);
    };
}

void NetworkClient::removeTransportHandlers() {
    Transport::activeTransport->onClientConnected = nullptr;
    Transport::activeTransport->onClientDisconnected = nullptr;
    Transport::activeTransport->onClientDataReceived = nullptr;
}

void NetworkClient::onTransportConnect() {
    KapEngine::Debug::log("NetworkClient: Connected");

    connectState = ConnectState::Connected;

    if (onConnectedEvent != nullptr) {
        onConnectedEvent();
    }
}

void NetworkClient::onTransportDisconnect() {
    KapEngine::Debug::log("NetworkClient: Disconnected");

    if (connectState == ConnectState::Disconnected) {
        return;
    }

    if (onDisconnectedEvent != nullptr) {
        onDisconnectedEvent();
    }

    connectState = ConnectState::Disconnected;

    removeTransportHandlers();
}

void NetworkClient::onTransportData(std::shared_ptr<ArraySegment<byte>> data) {
    KapEngine::Debug::log("NetworkClient: Data received");
}
