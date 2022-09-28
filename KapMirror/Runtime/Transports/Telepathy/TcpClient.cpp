#include "TcpClient.hpp"

#include <utility>
#include "SocketException.hpp"

using namespace KapMirror::Telepathy;

TcpClient::TcpClient(std::shared_ptr<Address> address, std::shared_ptr<Compression::ICompressionMethod> &compression) {
    if (address == nullptr) {
        throw std::runtime_error("Address cannot be null");
    }
    this->socket = Socket::createSocket(address);
    this->compression = compression;
    isConnected = false;
}

TcpClient::TcpClient(std::shared_ptr<Socket> socket, std::shared_ptr<Compression::ICompressionMethod> &compression) {
    if (socket == nullptr) {
        throw std::runtime_error("Socket cannot be null");
    }
    this->socket = socket;
    this->compression = compression;
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

void TcpClient::send(std::shared_ptr<ArraySegment<byte>> message) {
    if (!isConnected) {
        throw SocketException("Not connected");
    }
    Compression::ICompressionMethod *method = compression.get();

    if (method != nullptr) {
        ArraySegment<byte> *compressedSegment = method->compress(message).get();

        if (compressedSegment != nullptr) {
            socket->send(compressedSegment->toArray(), compressedSegment->getSize());
        }else{
            socket->send(message->toArray(), message->getSize());
        }
    }else{
        socket->send(message->toArray(), message->getSize());
    }
}

bool TcpClient::receive(int maxMessageSize, byte* buffer, int& size) {
    if (!isConnected) {
        throw SocketException("Not connected");
    }
    size = socket->receive(buffer, maxMessageSize);
    if (size <= 0) {
        return false;
    }

    Compression::ICompressionMethod *method = compression.get();
    ArraySegment<byte> *segment;

    if (method != nullptr) {
        segment = method->decompress(buffer, size).get();
    }else{
        segment = new ArraySegment<byte>(buffer, size);
    }
    if (segment != nullptr) {
        *buffer = *segment->toArray();
        size = segment->getSize();
    }else{
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

void TcpClient::setBlocking(bool blocking) {
    socket->setBlocking(blocking);
}
