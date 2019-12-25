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
    DrawingBuffer(const RenderRequest& renderRequest, const sp<ShaderBindings>& shaderBindings, uint32_t stride);
    DEFAULT_COPY_AND_ASSIGN(DrawingBuffer);

    VertexStream makeVertexStream(const RenderRequest& renderRequest, size_t length, size_t offset);

    const sp<ShaderBindings>& shaderBindings() const;

    const Buffer::Builder& vertices() const;
    Buffer::Builder& vertices();

    const Buffer::Snapshot& indices() const;
    void setIndices(Buffer::Snapshot indices);

    bool isInstanced() const;

    Buffer::Builder& getInstancedArrayBuilder(uint32_t divisor);

    std::vector<std::pair<uint32_t, Buffer::Snapshot>> makeDividedBufferSnapshots() const;

private:
    sp<ShaderBindings> _shader_bindings;
    sp<PipelineBindings> _pipeline_bindings;

    Buffer::Builder _vertices;
    std::map<uint32_t, Buffer::Builder> _divided_buffer_builders;

    Buffer::Snapshot _indices;
    bool _is_instanced;
};

}

#endif
