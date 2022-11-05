#include "Address.hpp"
#include <iostream>

using namespace KapMirror::Telepathy;

Address::Address(const std::string& host, int port) {
    addrinfo hints = {0};
    hints.ai_family = AF_INET; // Note: Windows fails to connect if left unspecified
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &address);
    if (status != 0) {
        throw std::runtime_error("getaddrinfo failed");
    }
}

Address::Address(int port, bool passive) {
    addrinfo hints = {0};
    hints.ai_family = AF_INET; // Note: Windows fails to connect if left unspecified
    hints.ai_socktype = SOCK_STREAM;
    if (passive) {
        hints.ai_flags = AI_PASSIVE;
    }

    int status = getaddrinfo(nullptr, std::to_string(port).c_str(), &hints, &address);
    if (status != 0) {
        throw std::runtime_error("getaddrinfo failed");
    }
}

Address::~Address() {
    if (address != nullptr) {
        freeaddrinfo(address);
    }
}

std::shared_ptr<Address> Address::createAddress(const std::string& host, int port) { return std::make_shared<Address>(host, port); }

std::shared_ptr<Address> Address::createAddress(int port, bool passive) { return std::make_shared<Address>(port, passive); }
