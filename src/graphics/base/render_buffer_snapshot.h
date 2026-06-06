#pragma once

#include "core/forwarding.h"

#include "renderer/base/buffer.h"

namespace ark {

struct RenderBufferSnapshot {

    struct UBOSnapshot {
        ByteArray::View _dirty_flags;
        ByteArray::View _buffer;
    };

    Vector<UBOSnapshot> _ubos;
    Vector<std::pair<uint32_t, Buffer::Snapshot>> _ssbos;
};

}