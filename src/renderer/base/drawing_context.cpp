#include "renderer/base/drawing_context.h"

#include "graphics/base/camera.h"
#include "graphics/inf/render_command.h"

#include "renderer/base/shader.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/snippet_delegate.h"
#include "renderer/inf/pipeline.h"

namespace ark {

namespace {

class RenderCommandBind final : public RenderCommand {
public:
    RenderCommandBind(DrawingContext context, sp<Snippet::DrawDecorator> snippetDraw)
        : _context(std::move(context)), _snippet_draw(std::move(snippetDraw)) {
    }

    void draw(GraphicsContext& graphicsContext) override {
        const sp<Pipeline>& pipeline = _context._bindings->ensurePipeline(graphicsContext);
        _snippet_draw->preDraw(graphicsContext, _context);
        _snippet_draw->postDraw(graphicsContext, _context);
    }

private:
    DrawingContext _context;
    sp<Snippet::DrawDecorator> _snippet_draw;
};

class RenderCommandDraw final : public RenderCommand {
public:
    RenderCommandDraw(DrawingContext context, sp<Snippet::DrawDecorator> snippetDraw)
        : _context(std::move(context)), _snippet_draw(std::move(snippetDraw)) {
    }

    void draw(GraphicsContext& graphicsContext) override {
        DPROFILER_TRACE("DrawCommand");

        _context.upload(graphicsContext);

        const sp<Pipeline>& pipeline = _context._bindings->ensurePipeline(graphicsContext);
        _snippet_draw->preDraw(graphicsContext, _context);
        pipeline->draw(graphicsContext, _context);
        _snippet_draw->postDraw(graphicsContext, _context);
    }

private:
    DrawingContext _context;
    sp<Snippet::DrawDecorator> _snippet_draw;
};

}

DrawingContext::DrawingContext(sp<PipelineBindings> pipelineBindings, sp<RenderLayerSnapshot::BufferObject> bufferObject, Buffer::Snapshot vertices, Buffer::Snapshot indices, uint32_t drawCount, DrawingParams parameters, sp<Traits> attachments)
    : PipelineContext{std::move(pipelineBindings), std::move(bufferObject)}, _vertices(std::move(vertices)), _indices(std::move(indices)), _draw_count(drawCount), _parameters(std::move(parameters)), _attachments(attachments ? std::move(attachments) : _bindings->attachments())
{
}

sp<RenderCommand> DrawingContext::toRenderCommand(const RenderRequest& renderRequest)
{
    DCHECK(_bindings, "DrawingContext cannot be converted to RenderCommand more than once");
    sp<Snippet::DrawDecorator> drawEvents = _bindings->snippet()->makeDrawDecorator(renderRequest);
    return sp<RenderCommand>::make<RenderCommandDraw>(std::move(*this), std::move(drawEvents));
}

sp<RenderCommand> DrawingContext::toBindCommand(const RenderRequest& renderRequest)
{
    DCHECK(_bindings, "DrawingContext cannot be converted to RenderCommand more than once");
    sp<Snippet::DrawDecorator> drawEvents = _bindings->snippet()->makeDrawDecorator(renderRequest);
    return sp<RenderCommand>::make<RenderCommandBind>(std::move(*this), std::move(drawEvents));
}

void DrawingContext::upload(GraphicsContext& graphicsContext) const
{
    _vertices.upload(graphicsContext);
    CHECK(_vertices.id(), "Invaild VertexBuffer");
    if(_indices)
    {
        _indices.upload(graphicsContext);
        CHECK(_indices.id(), "Invaild IndexBuffer");
    }
}

}
