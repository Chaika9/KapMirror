#include "TcpClient.hpp"
#include "SocketException.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

namespace KapMirror {
    namespace Transports {
        TcpClient::TcpClient() : client_fd(-1), target_fd(-1), isOwner(true) {
        }

        TcpClient::TcpClient(int _client_fd, int _target_fd) : client_fd(_client_fd), target_fd(_target_fd), isOwner(false) {
            isConnected = true;
        }

        TcpClient::~TcpClient() {
            if (isConnected) {
                ::close(client_fd);
                if (isOwner) {
                    ::close(target_fd);
                }
            }
        }

        void TcpClient::connect(std::string host, int port) {
            if (isConnected) {
                throw SocketException("Socket already connected");
            }

            if ((target_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                throw SocketException("Socket creation error");
            }

            struct sockaddr_in address;
            address.sin_family = AF_INET;
            address.sin_port = htons(port);

            // convert IPv4 and IPv6 addresses from text to binary
            if (inet_pton(AF_INET, host.c_str(), &address.sin_addr) <= 0) {
                throw SocketException("Invalid address");
            }

            if ((client_fd = ::connect(target_fd, (struct sockaddr*)&address, sizeof(address))) < 0) {
                throw SocketException("Connection failed");
            }
            isConnected = true;
        }

        void TcpClient::close() {
            if (!isConnected) {
                return;
            }
            ::close(client_fd);
            if (isOwner) {
                ::close(target_fd);
            }
            isConnected = false;
        }

        void TcpClient::send(ArraySegment<char>& data) {
            if (!isConnected) {
                throw SocketException("Socket not connected");
            }

            std::cout << "TcpClient: sending data=" << data.getSize() << std::endl;
            char *buffer = data.toArray();
            if ((::send(target_fd, buffer, data.getSize(), 0)) < 0) {
                throw SocketException("Send failed");
            }
        }

        ArraySegment<char> TcpClient::receive(int size) {
            if (size <= 0) {
                throw std::invalid_argument("size must be greater than 0");
            }
            if (!isConnected) {
                throw SocketException("Socket not connected");
            }

            char *buffer = new char[size];
            int readSize = ::recv(target_fd, buffer, size, 0);
            std::cout << "TcpClient: Received " << readSize << " bytes" << std::endl;
            if (readSize <= 0) {
                isConnected = false;
                throw SocketException("Receive failed");
            }
            return ArraySegment<char>(buffer, readSize);
        }
    }
}
