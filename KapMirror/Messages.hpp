#pragma once

#include "Core/NetworkMessage.hpp"
#include "Vectors.hpp"
#include <memory>

namespace KapMirror {
    struct ObjectSpawnMessage : NetworkMessage {
        // networkId of new or existing object
        unsigned int networkId;
        // Sets hasAuthority on the spawned object
        bool isOwner;
        std::string sceneName;
        // Prefab name
        std::string prefabName;
        KapEngine::Tools::Vector3 position;
        KapEngine::Tools::Vector3 rotation;
        KapEngine::Tools::Vector3 scale;
        // Custom payload
        std::shared_ptr<ArraySegment<byte>> payload;

        void serialize(KapMirror::NetworkWriter& writer) override {
            writer.write(networkId);
            writer.write(isOwner);
            writer.writeString(sceneName);
            writer.writeString(prefabName);
            writer.write(position.getX());
            writer.write(position.getY());
            writer.write(position.getZ());
            writer.write(rotation.getX());
            writer.write(rotation.getY());
            writer.write(rotation.getZ());
            writer.write(scale.getX());
            writer.write(scale.getY());
            writer.write(scale.getZ());
            writer.write(payload->getSize());
            writer.writeBytes(payload->toArray(), payload->getOffset(), payload->getSize());
        }

        void deserialize(KapMirror::NetworkReader& reader) override {
            networkId = reader.read<unsigned int>();
            isOwner = reader.read<bool>();
            sceneName = reader.readString();
            prefabName = reader.readString();
            position.setX(reader.read<float>());
            position.setY(reader.read<float>());
            position.setZ(reader.read<float>());
            rotation.setX(reader.read<float>());
            rotation.setY(reader.read<float>());
            rotation.setZ(reader.read<float>());
            scale.setX(reader.read<float>());
            scale.setY(reader.read<float>());
            scale.setZ(reader.read<float>());

            int payloadSize = reader.read<int>();
            if (payloadSize > 0) {
                byte* segmentValue = reader.readBytes(payloadSize);
                payload = std::make_shared<ArraySegment<byte>>(segmentValue, 0, payloadSize);
                delete[] segmentValue;
            } else {
                payload = std::make_shared<ArraySegment<byte>>();
            }
        }
    };

    struct ObjectDestroyMessage : NetworkMessage {
        // networkId of existing object
        unsigned int networkId;

        void serialize(KapMirror::NetworkWriter& writer) override { writer.write(networkId); }

        void deserialize(KapMirror::NetworkReader& reader) override { networkId = reader.read<unsigned int>(); }
    };

    struct ObjectTransformMessage : NetworkMessage {
        // networkId of existing object
        unsigned int networkId;
        KapEngine::Tools::Vector3 position;
        KapEngine::Tools::Vector3 rotation;
        KapEngine::Tools::Vector3 scale;

        void serialize(KapMirror::NetworkWriter& writer) override {
            writer.write(networkId);
            writer.write(position.getX());
            writer.write(position.getY());
            writer.write(position.getZ());
            writer.write(rotation.getX());
            writer.write(rotation.getY());
            writer.write(rotation.getZ());
            writer.write(scale.getX());
            writer.write(scale.getY());
            writer.write(scale.getZ());
        }

        void deserialize(KapMirror::NetworkReader& reader) override {
            networkId = reader.read<unsigned int>();
            position.setX(reader.read<float>());
            position.setY(reader.read<float>());
            position.setZ(reader.read<float>());
            rotation.setX(reader.read<float>());
            rotation.setY(reader.read<float>());
            rotation.setZ(reader.read<float>());
            scale.setX(reader.read<float>());
            scale.setY(reader.read<float>());
            scale.setZ(reader.read<float>());
        }
    };
} // namespace KapMirror
