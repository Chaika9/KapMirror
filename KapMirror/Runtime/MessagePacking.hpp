#pragma once

#include "NetworkMessage.hpp"
#include <typeinfo>

namespace KapMirror {
    class MessagePacking {
        public:
        template<typename T>
        inline static ushort getId() {
            std::size_t type = typeid(T).hash_code();
            return (ushort)type;
        }

        template<typename T>
        inline static void pack(T& message, NetworkWriter& writer) {
            writer.write(getId<T>());

            // serialize message
            message.serialize(writer);
        }

        inline static void unpack(NetworkReader& reader, ushort& messageType) {
            messageType = reader.read<ushort>();
        }
    };
}
