#include "renderer/base/compute_context.h"

#include "graphics/inf/render_command.h"

#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/inf/pipeline.h"

namespace ark {

namespace {

class RenderCommandCompute : public RenderCommand {
public:
    RenderCommandCompute(ComputeContext context)
        : _context(std::move(context)) {
    }

    virtual void draw(GraphicsContext& graphicsContext) override {
        const sp<Pipeline> pipeline = _context._shader_bindings->getPipeline(graphicsContext);
        pipeline->compute(graphicsContext, _context);
    }

private:
    ComputeContext _context;

};

}

ComputeContext::ComputeContext(sp<ShaderBindings> shaderBindings, std::vector<RenderLayer::UBOSnapshot> ubo, std::vector<Buffer::Snapshot> ssbo, std::array<int32_t, 3> numWorkGroups)
    : _shader_bindings(std::move(shaderBindings)), _ubos(std::move(ubo)), _ssbo(std::move(ssbo)), _num_work_groups(numWorkGroups)
{
}

sp<RenderCommand> ComputeContext::toComputeCommand()
{
    DCHECK(_shader_bindings, "ComputeContext cannot be converted to RenderCommand more than once");
    return sp<RenderCommandCompute>::make(std::move(*this));
}

}
