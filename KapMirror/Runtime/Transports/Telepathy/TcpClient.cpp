#include "TcpClient.hpp"
#include "SocketException.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace KapMirror {
    namespace Transports {
        TcpClient::TcpClient() : client_fd(-1), server_fd(-1) {
        }

        TcpClient::TcpClient(int _client_fd, int _server_fd) : client_fd(_client_fd), server_fd(_server_fd) {
        }

        TcpClient::~TcpClient() {
            ::close(client_fd);
        }

        void TcpClient::connect(std::string host, int port) {
            if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                throw SocketException("Socket creation error");
            }

            struct sockaddr_in address;
            address.sin_family = AF_INET;
            address.sin_port = htons(port);

            // convert IPv4 and IPv6 addresses from text to binary
            if (inet_pton(AF_INET, host.c_str(), &address.sin_addr) <= 0) {
                throw SocketException("Invalid address");
            }

            if ((client_fd = ::connect(server_fd, (struct sockaddr*)&address, sizeof(address))) < 0) {
                throw SocketException("Connection failed");
            }
        }

        void TcpClient::close() {
            ::close(client_fd);
        }
    }
}
