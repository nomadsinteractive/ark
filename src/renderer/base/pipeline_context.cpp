#include "renderer/base/compute_context.h"

namespace ark {

PipelineContext::PipelineContext(sp<ShaderBindings> shaderBindings, std::vector<RenderLayerSnapshot::UBOSnapshot> ubo, std::vector<std::pair<uint32_t, Buffer::Snapshot>> ssbos)
    : _shader_bindings(std::move(shaderBindings)), _ubos(std::move(ubo)), _ssbos(std::move(ssbos))
{
}

}
