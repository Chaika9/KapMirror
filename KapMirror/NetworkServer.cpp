#include "KapEngine.hpp"
#include "Debug.hpp"
#include "NetworkServer.hpp"
#include "Runtime/Transport.hpp"

using namespace KapMirror;

NetworkServer::NetworkServer() {
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

    initialized = true;
}

void NetworkServer::listen(int maxConnections) {
    initialize();

    this->maxConnections = maxConnections;
    Transport::activeTransport->serverStart();
}
