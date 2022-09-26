#pragma once

#include "Runtime/ArraySegment.hpp"
#include "Runtime/Platform.hpp"
#include <memory>
#include <functional>

namespace KapMirror {
    enum ConnectState {
        None,
        // connecting between Connect() and OnTransportConnected()
        Connecting,
        Connected,
        // disconnecting between Disconnect() and OnTransportDisconnected()
        Disconnecting,
        Disconnected
    };

    class NetworkClient {
        ConnectState connectState;

        public:
        NetworkClient();
        ~NetworkClient() = default;

        void connect(std::string ip, int port);

        void disconnect();

        bool active() const {
            return connectState == ConnectState::Connecting || connectState == ConnectState::Connected;
        }

        bool isConnecting() const {
            return connectState == ConnectState::Connecting;
        }

        bool isConnected() const {
            return connectState == ConnectState::Connected;
        }

        private:
        void addTransportHandlers();
        void removeTransportHandlers();

        void onTransportConnect();
        void onTransportDisconnect();
        void onTransportData(std::shared_ptr<ArraySegment<byte>> data);

        public:
        std::function<void()> onConnectedEvent;
        std::function<void()> onDisconnectedEvent;
    };
}
