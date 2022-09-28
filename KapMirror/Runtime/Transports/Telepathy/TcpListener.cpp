#include "TcpListener.hpp"

using namespace KapMirror::Telepathy;

TcpListener::TcpListener(std::shared_ptr<Address> address) {
    if (address == nullptr) {
        throw std::runtime_error("Address cannot be null");
    }
    socket = Socket::createSocket(address);
}

TcpListener::~TcpListener() {
    socket->close();
}

void TcpListener::close() {
    socket->close();
}

void TcpListener::start() {
    socket->bind();
    socket->listen();
}

std::shared_ptr<TcpClient> TcpListener::acceptTcpClient(std::shared_ptr<Compression::ICompressionMethod> compression) {
    auto clientSocket = socket->accept();
    if (clientSocket->isInvalid()) {
        throw SocketException("Invalid socket");
    }
    return std::make_shared<TcpClient>(clientSocket, compression);
}

bool TcpListener::isReadable() const {
    return socket->isReadable();
}

bool TcpListener::isWritable() const {
    return socket->isWritable();
}

void TcpListener::setBlocking(bool blocking) {
    socket->setBlocking(blocking);
}
