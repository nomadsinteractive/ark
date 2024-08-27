#include "renderer/base/compute_context.h"

#include "graphics/inf/render_command.h"

#include "renderer/base/shader.h"
#include "renderer/base/pipeline_bindings.h"

namespace ark {

ComputeContext::ComputeContext(sp<PipelineBindings> pipelineBindings, sp<RenderLayerSnapshot::BufferObject> bufferObject, std::array<uint32_t, 3> numWorkGroups)
    : PipelineContext{std::move(pipelineBindings), std::move(bufferObject)}, _num_work_groups(numWorkGroups)
{
}

}
