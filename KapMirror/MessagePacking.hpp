#pragma once

#include "NetworkMessage.hpp"

namespace KapMirror {
    class MessagePacking {
        public:
        inline static void pack(NetworkMessage& message, NetworkWriter& writer) {
            std::size_t type = typeid(message).hash_code();
            writer.write((ushort)type);

            // serialize message
            message.serialize(writer);
        }

        inline static void unpack(NetworkReader& reader, ushort& messageType, NetworkMessage& message) {
            messageType = reader.read<ushort>();

            // deserialize message
            message.deserialize(reader);
        }
    };
}
