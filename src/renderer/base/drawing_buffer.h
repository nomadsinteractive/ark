#ifndef ARK_RENDERER_BASE_DRAWING_BUFFER_H_
#define ARK_RENDERER_BASE_DRAWING_BUFFER_H_

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
    DrawingBuffer(const sp<ShaderBindings>& shaderBindings, uint32_t stride);
    DEFAULT_COPY_AND_ASSIGN(DrawingBuffer);

    VertexWriter makeVertexStream(const RenderRequest& renderRequest, size_t length, size_t offset);
    [[deprecated]]
    VertexWriter makeDividedVertexStream(const RenderRequest& renderRequest, size_t length, size_t offset, uint32_t divisor);

    const sp<ShaderBindings>& shaderBindings() const;

    const Buffer::Factory& vertices() const;
    Buffer::Factory& vertices();

    const Buffer::Snapshot& indices() const;
    void setIndices(Buffer::Snapshot indices);

    bool isInstanced() const;

    Buffer::Factory& getInstancedBufferBuilder(uint32_t divisor);

    std::vector<std::pair<uint32_t, Buffer::Snapshot>> toDividedBufferSnapshots();

private:
    sp<ShaderBindings> _shader_bindings;
    sp<PipelineBindings> _pipeline_bindings;

    Buffer::Factory _vertices;
    std::map<uint32_t, Buffer::Factory> _divided_buffer_builders;

    Buffer::Snapshot _indices;
    bool _is_instanced;
};

}

#endif
