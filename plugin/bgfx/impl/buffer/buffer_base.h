#pragma once

#include "renderer/base/buffer.h"

#include "bgfx/forwarding.h"
#include "bgfx/base/handle.h"

namespace ark::plugin::bgfx {

class BufferBase : public Buffer::Delegate {
public:
    BufferBase(Buffer::Type type, Buffer::Usage usage);

    virtual void bind() = 0;

    Buffer::Type type() const;
    Buffer::Usage usage() const;

protected:
    static void setupVertexBufferLayout(::bgfx::VertexLayout& vertexBufLayout, const PipelineDescriptor& pipelineDescriptor);

protected:
    Buffer::Type _type;
    Buffer::Usage _usage;
};

}
