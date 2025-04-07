#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
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

    const Buffer::SnapshotFactory& vertices() const;
    Buffer::SnapshotFactory& vertices();

    bool isInstanced() const;

    Buffer::SnapshotFactory& getDividedBufferBuilder(uint32_t divisor);

    std::vector<std::pair<uint32_t, Buffer::Snapshot>> toDividedBufferSnapshots();

private:
    sp<PipelineBindings> _pipeline_bindings;
    sp<PipelineDescriptor> _pipeline_descriptor;

    Buffer::SnapshotFactory _vertices;
    Map<uint32_t, Buffer::SnapshotFactory> _divided_buffer_builders;

    bool _is_instanced;
};

}
