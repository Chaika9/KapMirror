#pragma once

#include "Runtime/NetworkMessage.hpp"

namespace KapMirror {
    struct ObjectSpawnMessage : NetworkMessage {
        unsigned int networkId;
        bool isOwner;
        float x;
        float y;
        float z;

        void serialize(KapMirror::NetworkWriter& writer) {
            writer.write(networkId);
            writer.write(isOwner);
            writer.write(x);
            writer.write(y);
            writer.write(z);
        }

        void deserialize(KapMirror::NetworkReader& reader) {
            networkId = reader.read<unsigned int>();
            isOwner = reader.read<bool>();
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
