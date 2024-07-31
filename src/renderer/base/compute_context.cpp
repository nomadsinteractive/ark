#include "renderer/base/compute_context.h"

#include "graphics/inf/render_command.h"

#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/inf/pipeline.h"

namespace ark {

namespace {

class RenderCommandCompute final : public RenderCommand {
public:
    RenderCommandCompute(ComputeContext context)
        : _context(std::move(context)) {
    }

    void draw(GraphicsContext& graphicsContext) override {
        const sp<Pipeline>& pipeline = _context._pipeline_context._bindings->getPipeline(graphicsContext);
        pipeline->compute(graphicsContext, _context);
    }

private:
    ComputeContext _context;
};

}

ComputeContext::ComputeContext(PipelineSnapshot pipelineContext, std::array<int32_t, 3> numWorkGroups)
    : _pipeline_context(std::move(pipelineContext)), _num_work_groups(std::move(numWorkGroups))
{
}

}
