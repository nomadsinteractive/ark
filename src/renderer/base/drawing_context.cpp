#include "renderer/base/drawing_context.h"

#include "graphics/base/camera.h"
#include "graphics/inf/render_command.h"

#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/snippet_delegate.h"
#include "renderer/inf/pipeline.h"

namespace ark {

namespace {

class RenderCommandBind : public RenderCommand {
public:
    RenderCommandBind(DrawingContext context)
        : _context(std::move(context)) {
    }

    virtual void draw(GraphicsContext& graphicsContext) override {
        const sp<Pipeline> pipeline = _context._shader_bindings->getPipeline(graphicsContext);
        pipeline->bind(graphicsContext, _context);
    }

private:
    DrawingContext _context;
};

class RenderCommandDraw : public RenderCommand {
public:
    RenderCommandDraw(DrawingContext context, sp<Snippet::DrawEvents> snippetDraw)
        : _context(std::move(context)), _snippet_draw(std::move(snippetDraw)) {
    }

    virtual void draw(GraphicsContext& graphicsContext) override {
        _context.upload(graphicsContext);

        const sp<Pipeline> pipeline = _context._shader_bindings->getPipeline(graphicsContext);
        _snippet_draw->preDraw(graphicsContext, _context);
        pipeline->bind(graphicsContext, _context);
        pipeline->draw(graphicsContext, _context);
        _snippet_draw->postDraw(graphicsContext);
    }

private:
    DrawingContext _context;
    sp<Snippet::DrawEvents> _snippet_draw;
};

}


DrawingContext::DrawingContext(sp<ShaderBindings> shaderBindings, sp<ByType> attachments, std::vector<RenderLayerSnapshot::UBOSnapshot> ubo, std::vector<std::pair<uint32_t, Buffer::Snapshot> > ssbos)
    : PipelineContext(std::move(shaderBindings), std::move(ubo), std::move(ssbos)), _attachments(std::move(attachments))
{
}

DrawingContext::DrawingContext(sp<ShaderBindings> shaderBindings, sp<ByType> attachments, std::vector<RenderLayerSnapshot::UBOSnapshot> ubo, std::vector<std::pair<uint32_t, Buffer::Snapshot> > ssbos, Buffer::Snapshot vertices, Buffer::Snapshot indices, DrawingContextParams::Parameters parameters)
    : PipelineContext(std::move(shaderBindings), std::move(ubo), std::move(ssbos)), _attachments(std::move(attachments)), _vertices(std::move(vertices)), _indices(std::move(indices)), _parameters(std::move(parameters))
{
}

sp<RenderCommand> DrawingContext::toRenderCommand(const RenderRequest& renderRequest)
{
    DCHECK(_shader_bindings, "DrawingContext cannot be converted to RenderCommand more than once");
    return sp<RenderCommand>::make<RenderCommandDraw>(std::move(*this), _shader_bindings->snippet()->makeDrawEvents(renderRequest));
}

sp<RenderCommand> DrawingContext::toBindCommand()
{
    DCHECK(_shader_bindings, "DrawingContext cannot be converted to RenderCommand more than once");
    return sp<RenderCommandBind>::make(std::move(*this));
}

void DrawingContext::upload(GraphicsContext& graphicsContext)
{
    _vertices.upload(graphicsContext);
    DCHECK(_vertices.id(), "Invaild VertexBuffer");
    if(_indices)
    {
        _indices.upload(graphicsContext);
        DCHECK(_indices.id(), "Invaild IndexBuffer");
    }
}

}
