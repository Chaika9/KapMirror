#include "NetworkServer.hpp"
#include "Runtime/Transport.hpp"
#include "KapEngine.hpp"
#include "Debug.hpp"

using namespace KapMirror;

NetworkServer::NetworkServer() {
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

void NetworkServer::listen(int maxConnections) {
    initialize();

    this->maxConnections = maxConnections;
    Transport::activeTransport->serverStart();

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

    if (connectionExists(connectionId)) {
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
}

void NetworkServer::onTransportDisconnect(int connectionId) {
    KapEngine::Debug::log("NetworkServer: Client " + std::to_string(connectionId) + " disconnected");

    std::shared_ptr<NetworkConnectionToClient> connection;
    if (tryGetConnection(connectionId, connection)) {
        removeConnection(connectionId);
    }
}

void NetworkServer::onTransportData(int connectionId, std::shared_ptr<ArraySegment<byte>> data) {
    KapEngine::Debug::log("NetworkServer: Client " + std::to_string(connectionId) + " sent data");
}

bool NetworkServer::addConnection(std::shared_ptr<NetworkConnectionToClient> connection) {
    // TODO: Optimize this with Map or Dictionary
    for (auto& conn : connections) {
        if (conn->getConnectionId() == connection->getConnectionId()) {
            return false;
        }
    }
    connections.push_back(connection);
    return true;
}

bool NetworkServer::removeConnection(int connectionId) {
    for (auto it = connections.begin(); it != connections.end(); it++) {
        std::shared_ptr<NetworkConnectionToClient> conn = *it;
        if (conn->getConnectionId() == connectionId) {
            connections.erase(it);
            return true;
        }
    }
    return false;
}

bool NetworkServer::tryGetConnection(int connectionId, std::shared_ptr<NetworkConnectionToClient>& connection) {
    for (auto& conn : connections) {
        if (conn->getConnectionId() == connectionId) {
            connection = conn;
            return true;
        }
    }
    return false;
}

bool NetworkServer::connectionExists(int connectionId) {
    for (auto& conn : connections) {
        if (conn->getConnectionId() == connectionId) {
            return true;
        }
    }
    return false;
}

void NetworkServer::sendToAll(std::shared_ptr<KapMirror::ArraySegment<byte>> data) {
    if (!active) {
        KapEngine::Debug::warning("NetworkServer: Cannot send data, server not active");
        return;
    }
    for (auto& conn : connections) {
        conn->send(data);
    }
}

void NetworkServer::disconnectAll() {
    for (auto& conn : connections) {
        conn->disconnect();
        onTransportDisconnect(conn->getConnectionId());
    }

    // cleanup
    connections.clear();
}
