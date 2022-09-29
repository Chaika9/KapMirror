#pragma once

#include "KapMirror/Runtime/NetworkWriter.hpp"
#include "KapMirror/Runtime/NetworkReader.hpp"

namespace KapMirror {
    struct NetworkMessage {
        virtual void serialize(KapMirror::NetworkWriter& writer) = 0;
        virtual void deserialize(KapMirror::NetworkReader& reader) = 0;
    };
}
