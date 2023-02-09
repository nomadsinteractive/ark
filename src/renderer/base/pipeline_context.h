#ifndef ARK_RENDERER_BASE_PIPELINE_CONTEXT_H_
#define ARK_RENDERER_BASE_PIPELINE_CONTEXT_H_

#include <vector>

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/render_layer_snapshot.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"

namespace ark {

class ARK_API PipelineContext {
public:
    PipelineContext(sp<ShaderBindings> shaderBindings, std::vector<RenderLayerSnapshot::UBOSnapshot> ubo, std::vector<std::pair<uint32_t, Buffer::Snapshot>> ssbos);

    sp<ShaderBindings> _shader_bindings;

    std::vector<RenderLayerSnapshot::UBOSnapshot> _ubos;
    std::vector<std::pair<uint32_t, Buffer::Snapshot>> _ssbos;
};

}

#endif
