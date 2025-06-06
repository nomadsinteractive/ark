#include "renderer/impl/renderer/compute_pass.h"

#include "graphics/base/render_request.h"

#include "renderer/base/drawing_context.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/pipeline_bindings.h"

namespace ark {

ComputePass::ComputePass(sp<Shader> shader)
    : _shader(std::move(shader)), _pipeline_bindings(_shader->makeBindings(Buffer(), enums::DRAW_MODE_NONE, enums::DRAW_PROCEDURE_AUTO, Vector<std::pair<uint32_t, Buffer>>()))
{
}

void ComputePass::render(RenderRequest& renderRequest, const V3& /*position*/, const sp<DrawDecorator>& drawDecorator)
{
    DrawingContext drawingContext(_pipeline_bindings, _shader->takeBufferSnapshot(renderRequest, true));
    renderRequest.addRenderCommand(drawingContext.toNoopCommand(renderRequest, drawDecorator));
}

ComputePass::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _shader(factory.ensureBuilder<Shader>(manifest, constants::SHADER))
{
}

sp<Renderer> ComputePass::BUILDER::build(const Scope& args)
{
    return sp<Renderer>::make<ComputePass>(_shader->build(args));
}

}
