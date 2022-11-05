#include "NetworkTransform.hpp"
#include "KapMirror/Messages.hpp"
#include "Debug.hpp"
#include "Transform.hpp"

using namespace KapMirror;

NetworkTransform::NetworkTransform(std::shared_ptr<KapEngine::GameObject> go) : NetworkComponent(go, "NetworkTransform") {}

void NetworkTransform::setClientAuthority(bool _clientAuthority) { clientAuthority = _clientAuthority; }

void NetworkTransform::setActiveUpdate(bool _activeUpdate) { activeUpdate = _activeUpdate; }

void NetworkTransform::setActiveLateUpdate(bool _activeLateUpdate) { activeLateUpdate = _activeLateUpdate; }

void NetworkTransform::setSendRate(int rate) { sendRate = rate; }

void NetworkTransform::setLateUpdateDelay(int delay) { lateUpdateDelay = delay; }

void NetworkTransform::onUpdate() {
    if (isServer()) {
        updateServer();
    } else if (isClient()) {
        updateClient();
    }
}

#pragma region Server

void NetworkTransform::updateServer() {
    if (sendRate <= 0 || sendRate > 120) {
        throw std::runtime_error("Send rate must be between 1 and 120");
    }
    if (lateUpdateDelay < 300) {
        throw std::runtime_error("Late update delay must be greater than 300");
    }

    // Update transform
    if (activeUpdate && KapMirror::NetworkTime::localTime() - lastUpdateRefreshTime > 1000 / sendRate &&
        (!clientAuthority || isClientWithAuthority())) {
        lastUpdateRefreshTime = NetworkTime::localTime();
        lastLateUpdateRefreshTime = NetworkTime::localTime();

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
        return;
    }

    // Update late transform
    if (activeLateUpdate && KapMirror::NetworkTime::localTime() - lastLateUpdateRefreshTime > lateUpdateDelay &&
        (!clientAuthority || isClientWithAuthority())) {
        lastLateUpdateRefreshTime = NetworkTime::localTime();

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

#pragma endregion

#pragma region Client

void NetworkTransform::updateClient() {
    if (!isClientWithAuthority()) {
        return;
    }

    if (activeUpdate && KapMirror::NetworkTime::localTime() - lastUpdateRefreshTime > 1000 / sendRate) {
        lastUpdateRefreshTime = NetworkTime::localTime();
        // TODO: Implement client authority
    }
}

#pragma endregion
