#pragma once

#include "KapMirror/Runtime/Compression/ICompressionMethod.hpp"
#include "TcpClient.hpp"
#include "SocketException.hpp"

namespace KapMirror {
namespace Telepathy {
    class TcpListener {
        private:
        std::shared_ptr<Socket> socket;

        public:
        TcpListener(std::shared_ptr<Address> address);
        ~TcpListener();

        void close();

        void start();

        std::shared_ptr<TcpClient> acceptTcpClient(std::shared_ptr<Compression::ICompressionMethod>);

        bool isReadable() const;

        bool isWritable() const;

        void setBlocking(bool blocking);
    };
}
}
