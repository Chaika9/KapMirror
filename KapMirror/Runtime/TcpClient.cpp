#include "TcpClient.hpp"
#include "SocketException.hpp"
#include <iostream>

using namespace KapMirror;

TcpClient::TcpClient(std::shared_ptr<Address> address) {
    if (address == nullptr) {
        throw std::runtime_error("Address cannot be null");
    }
    socket = Socket::createSocket(address);
    isConnected = false;
}

TcpClient::TcpClient(std::shared_ptr<Socket> socket) {
    if (socket == nullptr) {
        throw std::runtime_error("Socket cannot be null");
    }
    this->socket = socket;
    isConnected = true;
}

TcpClient::~TcpClient() {
    socket->close();
}

void TcpClient::close() {
    socket->close();
    isConnected = false;
}

void TcpClient::connect() {
    if (isConnected) {
        throw SocketException("Already connected");
    }
    socket->connect();
    isConnected = true;
}

void TcpClient::send(ArraySegment<byte>& message) {
    if (!isConnected) {
        throw SocketException("Not connected");
    }
    socket->send(message.toArray(), message.getSize());
}

KapMirror::ArraySegment<byte> TcpClient::receive(int size) {
    if (!isConnected) {
        throw SocketException("Not connected");
    }
    byte* buffer = new byte[size];
    int received = socket->receive(buffer, size);
    ArraySegment<byte> message(buffer, received);
    delete[] buffer;
    return message;
}

bool TcpClient::isReadable() const {
    if (!isConnected) {
        return false;
    }
    return socket->isReadable();
}

bool TcpClient::isWritable() const {
    if (!isConnected) {
        return false;
    }
    return socket->isWritable();
}
