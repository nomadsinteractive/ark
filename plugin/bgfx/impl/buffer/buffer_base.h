#pragma once

#include "renderer/base/buffer.h"

#include "bgfx/forwarding.h"
#include "bgfx/base/handle.h"

namespace ark::plugin::bgfx {

class BufferBase : public Buffer::Delegate {
public:
    BufferBase(Buffer::Usage usages);

    virtual void bind() = 0;
    virtual void bindRange(uint32_t first, uint32_t count) = 0;

    Buffer::Usage usages() const;

protected:
    static void setupVertexBufferLayout(::bgfx::VertexLayout& vertexLayout, const PipelineDescriptor& pipelineDescriptor);
    static void shiftTexCoords(const ::bgfx::VertexLayout& vertexLayout, void* data, size_t numberOfVertex);

protected:
    Buffer::Usage _usages;
};

}
