#ifndef ARK_RENDERER_BASE_COMPUTE_CONTEXT_H_
#define ARK_RENDERER_BASE_COMPUTE_CONTEXT_H_

#include <vector>

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/render_layer.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"

namespace ark {

class ARK_API ComputeContext {
public:
    ComputeContext(const sp<ShaderBindings>& shaderBindings, std::vector<RenderLayer::UBOSnapshot> ubo, Buffer::Snapshot vertexBuffer, int32_t numWorkGroups);
    DEFAULT_COPY_AND_ASSIGN(ComputeContext);

    sp<RenderCommand> toComputeCommand();
    sp<RenderCommand> toBindCommand();

    sp<ShaderBindings> _shader_bindings;

    std::vector<RenderLayer::UBOSnapshot> _ubos;
    Buffer::Snapshot _vertex_buffer;
    int32_t _num_work_groups;

};

}

#endif
