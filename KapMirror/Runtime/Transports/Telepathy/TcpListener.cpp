#include "TcpListener.hpp"
#include "SocketException.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

namespace KapMirror {
    namespace Transports {
        TcpListener::TcpListener(int _port) : port(_port) {
            if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                std::cout << "Socket creation error" << std::endl;
                return;
            }

            int opt = 1;
            if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
                std::cout << "Socket options error" << std::endl;
                return;
            }
        }

        TcpListener::~TcpListener() {
            close(server_fd);
        }

        void TcpListener::start() {
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
        }

        void TcpListener::stop() {
            close(server_fd);
        }

        std::unique_ptr<TcpClient> TcpListener::acceptTcpClient() {
            int client_fd;
            struct sockaddr_in address;
            int addrlen = sizeof(address);

            if ((client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
                throw SocketException("Socket accept error");
            }
            return std::make_unique<TcpClient>(client_fd);
        }
    }
}
