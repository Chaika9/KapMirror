#pragma once

#include "NetworkWriter.hpp"
#include "NetworkReader.hpp"

namespace KapMirror {
    struct NetworkMessage {
        virtual void serialize(KapMirror::NetworkWriter& writer) = 0;
        virtual void deserialize(KapMirror::NetworkReader& reader) = 0;
    };
}
