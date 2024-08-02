#pragma once

#include "renderer/base/buffer.h"

#include "bgfx/forwarding.h"
#include "bgfx/base/handle.h"

namespace ark::plugin::bgfx {

class BufferBase : public Buffer::Delegate {
public:

    uint64_t id() override;

    ResourceRecycleFunc recycle() override;

private:
    Buffer::Usage _usage;
    std::variant<StaticVertexBuffer, DynamicVertexBuffer> _handle;
};

}
