#include "NetworkTransform.hpp"
#include "KapMirror/Messages.hpp"
#include "Debug.hpp"
#include "Transform.hpp"

using namespace KapMirror;

NetworkTransform::NetworkTransform(std::shared_ptr<KapEngine::GameObject> go) : NetworkComponent(go, "NetworkTransform") {
}

void NetworkTransform::setClientAuthority(bool _clientAuthority) {
    clientAuthority = _clientAuthority;
}

void NetworkTransform::setSendRate(int _sendRate) {
    sendRate = _sendRate;
}

void NetworkTransform::onUpdate() {
    if (isServer()) {
        updateServer();
    } else if (isClient()) {
        updateClient();
    }
}

void NetworkTransform::updateServer() {
    if (KapMirror::NetworkTime::localTime() - lastRefreshTime > 1000 / sendRate &&
        (!clientAuthority || isClientWithAuthority())) {
        lastRefreshTime = NetworkTime::localTime();

        auto& transform = getGameObject().getComponent<KapEngine::Transform>();
        if (transform.getLocalPosition() == lastPosition) {
            return;
        }
        lastPosition = transform.getLocalPosition();

        ObjectTransformMessage message;
        message.networkId = networkIdentity->getNetworkId();
        message.x = transform.getLocalPosition().getX();
        message.y = transform.getLocalPosition().getY();
        message.z = transform.getLocalPosition().getZ();
        getServer()->sendToAll(message);
    }
}

void NetworkTransform::updateClient() {
    if (!isClientWithAuthority()) {
        return;
    }

    if (KapMirror::NetworkTime::localTime() - lastRefreshTime > 1000 / sendRate) {
        lastRefreshTime = NetworkTime::localTime();
        // TODO: Implement client authority
    }
}
