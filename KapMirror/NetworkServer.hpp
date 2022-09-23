#pragma once

namespace KapMirror {
    class NetworkServer {
        private:
        bool initialized;

        int maxConnections;

        public:
        NetworkServer();
        ~NetworkServer() = default;

        void listen(int maxConnections);

        private:
        void initialize();
    };
}
