#pragma once

#include "NetworkMessage.hpp"
#include <typeinfo>

namespace KapMirror {
    class MessagePacking {
      public:
        /**
         * @brief Gerenate a ID from a message type
         * @tparam T The message type
         * @return ID of the message type
         */
        template <typename T>
        inline static ushort getId() {
            return (ushort)std::hash<std::string>()(typeid(T).name());
        }

        /**
         * @brief Pack a message to a byte array with the message ID
         * @tparam T The message type
         * @param message The message
         * @param writer The NetworkWriter
         */
        template <typename T>
        inline static void pack(T& message, NetworkWriter& writer) {
            writer.write(getId<T>());

            // serialize message
            message.serialize(writer);
        }

        /**
         * @brief Unpack a message from a byte array
         * @tparam T The message type
         * @param reader The NetworkReader
         * @param messageType The message type
         */
        inline static void unpack(NetworkReader& reader, ushort& messageType) { messageType = reader.read<ushort>(); }
    };
} // namespace KapMirror
