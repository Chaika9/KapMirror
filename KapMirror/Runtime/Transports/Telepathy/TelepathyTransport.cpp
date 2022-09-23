#include "TelepathyTransport.hpp"
#include <iostream>

using namespace KapMirror;

TelepathyTransport::TelepathyTransport() {
}

TelepathyTransport::~TelepathyTransport() {
    if (server) {
        server->close();
    }
}

/**
 * Client
 */

void TelepathyTransport::createClient() {
    std::cout << "TelepathyTransport: Creating client" << std::endl;

    client = std::make_shared<Telepathy::Client>(1024);
    client->onConnected = [this]() {
        if (onClientConnected) {
            onClientConnected(*this);
        }
    };
    client->onDisconnected = [this]() {
        if (onClientDisconnected) {
            onClientDisconnected(*this);
        }
    };
    client->onData = [this](std::shared_ptr<ArraySegment<byte>> data) {
        if (onClientDataReceived) {
            onClientDataReceived(*this, data);
        }
    };
}

bool TelepathyTransport::clientConnected() {
    return client != nullptr && client->connected();
}

void TelepathyTransport::clientConnect(std::string ip, int port) {
    createClient();
    std::cout << "TelepathyTransport: Connecting to " << ip << ":" << port << std::endl;
    client->connect(ip, port);
}

void TelepathyTransport::clientDisconnect() {
    std::cout << "TelepathyTransport: Disconnecting" << std::endl;
    if (client) {
        client->disconnect();
    }
}

void TelepathyTransport::clientSend(std::shared_ptr<ArraySegment<byte>> data) {
    if (client) {
        client->send(data);
    }
}

void TelepathyTransport::clientEarlyUpdate() {
    if (client) {
        client->tick(100);
    }
}

/**
 * Server
 */

void TelepathyTransport::serverStart() {
    std::cout << "TelepathyTransport: Starting server" << std::endl;

    // Create server
    server = std::make_shared<Telepathy::Server>(1024);

    // Servers Hooks
    server->onConnected = [this](int connectionId) {
        if (onServerConnected) {
            onServerConnected(*this, connectionId);
        }
    };
    server->onDisconnected = [this](int connectionId) {
        if (onServerDisconnected) {
            onServerDisconnected(*this, connectionId);
        }
    };
    server->onData = [this](int connectionId, std::shared_ptr<ArraySegment<byte>> data) {
        if (onServerDataReceived) {
            onServerDataReceived(*this, connectionId, data);
        }
    };

    server->start(25565);
}

void TelepathyTransport::serverStop() {
    if (server) {
        server->close();
    }
}

void TelepathyTransport::serverSend(int connectionId, std::shared_ptr<ArraySegment<byte>> data) {
    if (server != nullptr) {
        server->send(connectionId, data);
    }
}

void TelepathyTransport::serverEarlyUpdate() {
    if (server != nullptr) {
        server->tick(100);
    }
}
