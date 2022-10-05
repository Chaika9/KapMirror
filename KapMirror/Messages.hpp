#pragma once

#include "Runtime/NetworkMessage.hpp"

namespace KapMirror {
    struct ObjectSpawnMessage : NetworkMessage {
        // networkId of new or existing object
        unsigned int networkId;
        // Sets hasAuthority on the spawned object
        bool isOwner;
        std::size_t sceneId;
        // Prefab name
        std::string prefabName;
        float x;
        float y;
        float z;

        void serialize(KapMirror::NetworkWriter& writer) {
            writer.write(networkId);
            writer.write(isOwner);
            writer.write(sceneId);
            writer.writeString(prefabName);
            writer.write(x);
            writer.write(y);
            writer.write(z);
        }

        void deserialize(KapMirror::NetworkReader& reader) {
            networkId = reader.read<unsigned int>();
            isOwner = reader.read<bool>();
            sceneId = reader.read<std::size_t>();
            prefabName = reader.readString();
            x = reader.read<float>();
            y = reader.read<float>();
            z = reader.read<float>();
        }
    };

    struct ObjectDestroyMessage : NetworkMessage {
        unsigned int networkId;

        void serialize(KapMirror::NetworkWriter& writer) {
            writer.write(networkId);
        }

        void deserialize(KapMirror::NetworkReader& reader) {
            networkId = reader.read<unsigned int>();
        }
    };

    struct ObjectTransformMessage : NetworkMessage {
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
