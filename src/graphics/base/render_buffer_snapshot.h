#pragma once

#include "core/forwarding.h"

#include "renderer/base/buffer.h"

namespace ark {

struct RenderBufferSnapshot {

    struct UBOSnapshot {
        ByteArray::Borrowed _dirty_flags;
        ByteArray::Borrowed _buffer;
    };

    Vector<UBOSnapshot> _ubos;
    Vector<std::pair<uint32_t, Buffer::Snapshot>> _ssbos;
};

}