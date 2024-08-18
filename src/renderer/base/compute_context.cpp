#include "renderer/base/compute_context.h"

#include "graphics/inf/render_command.h"

#include "renderer/base/shader.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/inf/pipeline.h"

namespace ark {

namespace {

class RenderCommandCompute final : public RenderCommand {
public:
    RenderCommandCompute(ComputeContext context)
        : _context(std::move(context)) {
    }

    void draw(GraphicsContext& graphicsContext) override {
        const sp<Pipeline>& pipeline = _context._bindings->getPipeline(graphicsContext);
        pipeline->compute(graphicsContext, _context);
    }

private:
    ComputeContext _context;
};

}

ComputeContext::ComputeContext(sp<PipelineBindings> pipelineBindings, sp<RenderLayerSnapshot::BufferObject> bufferObject, std::array<int32_t, 3> numWorkGroups)
    : PipelineContext{std::move(pipelineBindings), std::move(bufferObject)}, _num_work_groups(std::move(numWorkGroups))
{
}

}
