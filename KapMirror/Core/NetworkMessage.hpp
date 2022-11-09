#pragma once

#include "NetworkWriter.hpp"
#include "NetworkReader.hpp"

namespace KapMirror {
    struct NetworkMessage {
        /**
         * @brief Serialize the message into a NetworkWriter
         * @param writer The NetworkWriter
         */
        virtual void serialize(KapMirror::NetworkWriter& writer) = 0;

        /**
         * @brief Deserialize the message from a NetworkReader
         * @param reader The NetworkReader
         */
        virtual void deserialize(KapMirror::NetworkReader& reader) = 0;
    };
} // namespace KapMirror
