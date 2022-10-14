#pragma once

#include "Runtime/NetworkMessage.hpp"
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
        float x;
        float y;
        float z;
        // Custom payload
        std::shared_ptr<ArraySegment<byte>> payload;

        void serialize(KapMirror::NetworkWriter& writer) {
            writer.write(networkId);
            writer.write(isOwner);
            writer.writeString(sceneName);
            writer.writeString(prefabName);
            writer.write(x);
            writer.write(y);
            writer.write(z);
            writer.write(payload->getSize());
            writer.writeBytes(payload->toArray(), payload->getOffset(), payload->getSize());
        }

        void deserialize(KapMirror::NetworkReader& reader) {
            networkId = reader.read<unsigned int>();
            isOwner = reader.read<bool>();
            sceneName = reader.readString();
            prefabName = reader.readString();
            x = reader.read<float>();
            y = reader.read<float>();
            z = reader.read<float>();

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

        void serialize(KapMirror::NetworkWriter& writer) {
            writer.write(networkId);
        }

        void deserialize(KapMirror::NetworkReader& reader) {
            networkId = reader.read<unsigned int>();
        }
    };

    struct ObjectTransformMessage : NetworkMessage {
        // networkId of existing object
        unsigned int networkId;
        float x;
        float y;
        float z;

        void serialize(KapMirror::NetworkWriter& writer) {
            writer.write(networkId);
            writer.write(x);
            writer.write(y);
            writer.write(z);
        }

        void deserialize(KapMirror::NetworkReader& reader) {
            networkId = reader.read<unsigned int>();
            x = reader.read<float>();
            y = reader.read<float>();
            z = reader.read<float>();
        }
    };
}
