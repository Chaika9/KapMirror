#include "NetworkServer.hpp"
#include "Runtime/Transport.hpp"

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

void NetworkServer::disconnectAll() {
    for (auto const& [id, conn] : connections) {
        conn->disconnect();
        onTransportDisconnect(conn->getConnectionId());
    }

    // cleanup
    connections.clear();
}
