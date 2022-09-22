#pragma once

#include "TcpClient.hpp"
#include "SocketException.hpp"

namespace KapMirror {
    class TcpListener {
        private:
        std::shared_ptr<Socket> socket;

        public:
        TcpListener(std::shared_ptr<Address> address);
        ~TcpListener();

        void close();

        void start();

        std::shared_ptr<TcpClient> acceptTcpClient();

        bool isReadable() const;

        bool isWritable() const;

        void setBlocking(bool blocking);
    };
}
