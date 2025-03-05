#include "renderer/base/drawing_context.h"

#include "graphics/base/camera.h"
#include "graphics/inf/render_command.h"

#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/snippet_delegate.h"
#include "renderer/inf/pipeline.h"

namespace ark {

namespace {

class RenderCommandNoop final : public RenderCommand {
public:
    RenderCommandNoop(DrawingContext context, sp<Snippet::DrawDecorator> drawDecorator)
        : _context(std::move(context)), _draw_decorator(std::move(drawDecorator)) {
    }

    void draw(GraphicsContext& graphicsContext) override {
        _context._bindings->ensurePipeline(graphicsContext);
        _draw_decorator->preDraw(graphicsContext, _context);
        _draw_decorator->postDraw(graphicsContext, _context);
    }

private:
    DrawingContext _context;
    sp<Snippet::DrawDecorator> _draw_decorator;
};

class RenderCommandDraw final : public RenderCommand {
public:
    RenderCommandDraw(DrawingContext context, sp<Snippet::DrawDecorator> drawDecorator)
        : _context(std::move(context)), _draw_decorator(std::move(drawDecorator)) {
    }

    void draw(GraphicsContext& graphicsContext) override {
        DPROFILER_TRACE("DrawCommand");

        _context.upload(graphicsContext);

        const sp<Pipeline>& pipeline = _context._bindings->ensurePipeline(graphicsContext);
        _draw_decorator->preDraw(graphicsContext, _context);
        pipeline->draw(graphicsContext, _context);
        _draw_decorator->postDraw(graphicsContext, _context);
    }

private:
    DrawingContext _context;
    sp<Snippet::DrawDecorator> _draw_decorator;
};

}

DrawingContext::DrawingContext(sp<PipelineBindings> pipelineBindings, sp<RenderLayerSnapshot::BufferObject> bufferObject, Buffer::Snapshot vertices, Buffer::Snapshot indices, const uint32_t drawCount, DrawingParams parameters, const sp<Traits>& attachments)
    : PipelineContext{std::move(pipelineBindings), std::move(bufferObject)}, _vertices(std::move(vertices)), _indices(std::move(indices)), _draw_count(drawCount), _parameters(std::move(parameters)), _attachments(attachments ? attachments : _bindings->attachments())
{
}

sp<RenderCommand> DrawingContext::toRenderCommand(const RenderRequest& renderRequest)
{
    DCHECK(_bindings, "DrawingContext cannot be converted to RenderCommand more than once");
    sp<Snippet::DrawDecorator> drawDecorator = _bindings->snippet()->makeDrawDecorator(renderRequest);
    return sp<RenderCommand>::make<RenderCommandDraw>(std::move(*this), std::move(drawDecorator));
}

sp<RenderCommand> DrawingContext::toNoopCommand(const RenderRequest& renderRequest)
{
    DCHECK(_bindings, "DrawingContext cannot be converted to RenderCommand more than once");
    sp<Snippet::DrawDecorator> drawDecorator = _bindings->snippet()->makeDrawDecorator(renderRequest);
    return sp<RenderCommand>::make<RenderCommandNoop>(std::move(*this), std::move(drawDecorator));
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
