#pragma once

#include "TcpClient.hpp"
#include "SocketException.hpp"

namespace KapMirror::Telepathy {
    class TcpListener {
        private:
        std::shared_ptr<Socket> socket;

        public:
        TcpListener(std::shared_ptr<Address> address);
        ~TcpListener();

        /**
         * @brief Close the listener
         */
        void close();

        /**
         * @brief Start listening for connections
         */
        void start();

        /**
         * @brief Accept a connection
         *
         * @return The client socket
         */
        std::shared_ptr<TcpClient> acceptTcpClient();

        bool isReadable() const;

        bool isWritable() const;

        void setBlocking(bool blocking);
    };
}
