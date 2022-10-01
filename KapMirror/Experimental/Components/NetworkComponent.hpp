#pragma once

#include "Component.hpp"

namespace KapMirror {
namespace Experimental {
    class NetworkComponent : public KapEngine::Component {
        public:
        NetworkComponent(std::shared_ptr<KapEngine::GameObject> go, std::string name);
        ~NetworkComponent() = default;

        virtual void onStartServer() {}
        virtual void onStopServer() {}

        virtual void onStartClient() {}
        virtual void onStopClient() {}
    };
}
}
