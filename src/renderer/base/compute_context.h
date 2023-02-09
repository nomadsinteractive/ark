#ifndef ARK_RENDERER_BASE_COMPUTE_CONTEXT_H_
#define ARK_RENDERER_BASE_COMPUTE_CONTEXT_H_

#include <array>
#include <vector>

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/render_layer_snapshot.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/pipeline_context.h"

namespace ark {

class ARK_API ComputeContext : public PipelineContext {
public:
    ComputeContext(sp<ShaderBindings> shaderBindings, std::vector<RenderLayerSnapshot::UBOSnapshot> ubo, std::vector<std::pair<uint32_t, Buffer::Snapshot>> ssbo, std::array<int32_t, 3> numWorkGroups);
    DEFAULT_COPY_AND_ASSIGN(ComputeContext);

    sp<RenderCommand> toComputeCommand();

    std::array<int32_t, 3> _num_work_groups;
};

}

#endif
