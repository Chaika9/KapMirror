#include "NetworkServer.hpp"
#include "Runtime/Transport.hpp"
#include "KapEngine.hpp"
#include "Debug.hpp"

using namespace KapMirror;

NetworkServer::NetworkServer(std::shared_ptr<Compression::ICompressionMethod> &compression) {
    this->compression = compression;
    initialized = false;
}

void NetworkServer::initialize() {
    if (initialized) {
        return;
    }

    if (Transport::activeTransport == nullptr) {
        KapEngine::Debug::error("NetworkServer: No transport set, cannot initialize");
        throw std::runtime_error("No transport set");
    }

    addTransportHandlers();

    initialized = true;
}

void NetworkServer::listen(int maxConnections) {
    initialize();

    this->maxConnections = maxConnections;
    Transport::activeTransport->serverStart(this->compression);
}

void NetworkServer::shutdown() {
    if (initialized) {
        removeTransportHandlers();

        Transport::activeTransport->serverStop();

        initialized = false;
    }
}

void NetworkServer::networkEarlyUpdate() {
    if (initialized) {
        Transport::activeTransport->serverEarlyUpdate();
    }
}

void NetworkServer::addTransportHandlers() {
    Transport::activeTransport->onServerConnected = [this](Transport& t, int connectionId) {
        onTransportConnect(connectionId);
    };
    Transport::activeTransport->onServerDisconnected = [this](Transport& t, int connectionId) {
        onTransportDisconnect(connectionId);
    };
    Transport::activeTransport->onServerDataReceived = [this](Transport& t, int connectionId, std::shared_ptr<ArraySegment<byte>> data) {
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

    if (connectionId >= maxConnections) {
        Transport::activeTransport->serverDisconnect(connectionId);
        KapEngine::Debug::warning("NetworkServer: Server full, kicked client " + std::to_string(connectionId));
        return;
    }
}

void NetworkServer::onTransportDisconnect(int connectionId) {
    KapEngine::Debug::log("NetworkServer: Client " + std::to_string(connectionId) + " disconnected");
}

void NetworkServer::onTransportData(int connectionId, std::shared_ptr<ArraySegment<byte>> data) {
    KapEngine::Debug::log("NetworkServer: Client " + std::to_string(connectionId) + " sent data");
}
