#pragma once

#include "KapMirror/Runtime/ArraySegment.hpp"
#include <string>
#include <stdexcept>

namespace KapMirror {
    namespace Transports {
        class TcpClient {
            private:
            int client_fd;
            int target_fd;
            bool isOwner;

            bool isConnected = false;

            public:
            TcpClient();
            TcpClient(int _client_fd, int _server_fd);
            ~TcpClient();

            /**
             * @brief Connect to a server
             * @param host Hostname or IP address
             * @param port Port number
             * @throws SocketException
             */
            void connect(std::string host, int port);

            /**
             * @brief Close the connection
             */
            void close();

            /**
             * @brief Is the client connected to a server
             * @return True if connected
             */
            bool connected() const {
                return isConnected;
            }

            /**
             * @brief Send data to the server
             * @param data Data to send
             * @throws SocketException
             */
            void send(ArraySegment<char>& data);

            /**
             * @brief Receive data from the server
             * @param size Maximum size of the data to receive
             * @return Received data
             * @throws SocketException
             */
            ArraySegment<char> receive(int size);
        };
    }
}
