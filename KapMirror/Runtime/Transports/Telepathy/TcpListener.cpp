#include "TcpListener.hpp"
#include "SocketException.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

namespace KapMirror {
    namespace Transports {
        TcpListener::TcpListener(int _port) : port(_port), server_fd(-1) {
        }

        TcpListener::~TcpListener() {
            if (isConnected) {
                ::close(server_fd);
            }
        }

        void TcpListener::start() {
            if (isConnected) {
                throw SocketException("Socket already connected");
            }

            if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                throw SocketException("Socket creation error");
            }

            int opt = 1;
            if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
                throw SocketException("Socket options error");
            }

            struct sockaddr_in address;
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = INADDR_ANY;
            address.sin_port = htons(port);

            if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
                throw SocketException("Socket bind error");
            }

            if (listen(server_fd, 3) < 0) {
                throw SocketException("Socket listen error");
            }
            isConnected = true;
        }

        void TcpListener::stop() {
            if (!isConnected) {
                return;
            }
            close(server_fd);
            isConnected = false;
        }

        std::unique_ptr<TcpClient> TcpListener::acceptTcpClient() {
            if (!isConnected) {
                throw SocketException("Socket not connected");
            }

            int client_fd;
            struct sockaddr_in address;
            int addrlen = sizeof(address);

            if ((client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
                throw SocketException("Socket accept error");
            }
            return std::make_unique<TcpClient>(server_fd, client_fd);
        }
    }
}
