#include "TcpClient.hpp"
#include <unistd.h>

KapMirror::Transports::TcpClient::TcpClient(int _client_fd) : client_fd(_client_fd) {
}

KapMirror::Transports::TcpClient::~TcpClient() {
    ::close(client_fd);
}

void KapMirror::Transports::TcpClient::close() {
    ::close(client_fd);
}
