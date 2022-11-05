#include "TcpClient.hpp"
#include "SocketException.hpp"

using namespace KapMirror::Telepathy;

TcpClient::TcpClient(const std::shared_ptr<Address>& address) {
    if (address == nullptr) {
        throw std::runtime_error("Address cannot be null");
    }
    socket      = Socket::createSocket(address);
    isConnected = false;
}

TcpClient::TcpClient(const std::shared_ptr<Socket>& socket) {
    if (socket == nullptr) {
        throw std::runtime_error("Socket cannot be null");
    }
    this->socket = socket;
    isConnected  = true;
}

TcpClient::~TcpClient() { socket->close(); }

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

void TcpClient::send(byte* buffer, int size) {
    if (!isConnected) {
        throw SocketException("Not connected");
    }
    socket->send(buffer, size);
}

bool TcpClient::receive(int maxMessageSize, byte* buffer, int& size) {
    if (!isConnected) {
        throw SocketException("Not connected");
    }
    size = socket->receive(buffer, maxMessageSize);
    if (size <= 0) {
        return false;
    }
    return true;
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

void TcpClient::setBlocking(bool blocking) { socket->setBlocking(blocking); }
