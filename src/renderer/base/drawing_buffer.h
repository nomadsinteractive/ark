#pragma once

#include <vector>
#include <map>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/render_object.h"
#include "graphics/base/transform.h"
#include "graphics/base/v3.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/varyings.h"

namespace ark {

class ARK_API DrawingBuffer {
public:
    DrawingBuffer(sp<PipelineBindings> pipelineBindings, uint32_t stride);
    DEFAULT_COPY_AND_ASSIGN(DrawingBuffer);

    VertexWriter makeVertexWriter(const RenderRequest& renderRequest, size_t length, size_t offset);
    [[deprecated]]
    VertexWriter makeDividedVertexWriter(const RenderRequest& renderRequest, size_t length, size_t offset, uint32_t divisor);

    const sp<PipelineBindings>& pipelineBindings() const;

    const Buffer::Factory& vertices() const;
    Buffer::Factory& vertices();

    const Buffer::Snapshot& indices() const;
    void setIndices(Buffer::Snapshot indices);

    bool isInstanced() const;

    Buffer::Factory& getDividedBufferBuilder(uint32_t divisor);

    std::vector<std::pair<uint32_t, Buffer::Snapshot>> toDividedBufferSnapshots();

private:
    sp<PipelineBindings> _pipeline_bindings;
    sp<PipelineDescriptor> _pipeline_descriptor;

    Buffer::Factory _vertices;
    std::map<uint32_t, Buffer::Factory> _divided_buffer_builders;

    Buffer::Snapshot _indices;
    bool _is_instanced;
};

}
