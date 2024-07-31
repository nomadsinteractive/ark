#include "renderer/base/drawing_context.h"

#include "graphics/base/camera.h"
#include "graphics/inf/render_command.h"

#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/snippet_delegate.h"
#include "renderer/inf/pipeline.h"

namespace ark {

namespace {

class RenderCommandBind final : public RenderCommand {
public:
    RenderCommandBind(DrawingContext context)
        : _context(std::move(context)) {
    }

    void draw(GraphicsContext& graphicsContext) override {
        const sp<Pipeline> pipeline = _context._pipeline_snapshot._bindings->getPipeline(graphicsContext);
        pipeline->bind(graphicsContext, _context);
    }

private:
    DrawingContext _context;
};

class RenderCommandDraw final : public RenderCommand {
public:
    RenderCommandDraw(DrawingContext context, sp<Snippet::DrawEvents> snippetDraw)
        : _context(std::move(context)), _snippet_draw(std::move(snippetDraw)) {
    }

    void draw(GraphicsContext& graphicsContext) override {
        DPROFILER_TRACE("DrawCommand");

        _context.upload(graphicsContext);

        const sp<Pipeline> pipeline = _context._pipeline_snapshot._bindings->getPipeline(graphicsContext);
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

DrawingContext::DrawingContext(PipelineSnapshot pipelineSnapshot, sp<Traits> attachments)
    : _pipeline_snapshot(std::move(pipelineSnapshot)), _attachments(std::move(attachments))
{
}

DrawingContext::DrawingContext(PipelineSnapshot pipelineSnapshot, sp<Traits> attachments, Buffer::Snapshot vertices, Buffer::Snapshot indices, uint32_t drawCount, DrawingParams parameters)
    : _pipeline_snapshot(std::move(pipelineSnapshot)), _attachments(std::move(attachments)), _vertices(std::move(vertices)), _indices(std::move(indices)), _draw_count(drawCount), _parameters(std::move(parameters))
{
}

sp<RenderCommand> DrawingContext::toRenderCommand(const RenderRequest& renderRequest)
{
    DCHECK(_pipeline_snapshot._bindings, "DrawingContext cannot be converted to RenderCommand more than once");
    return sp<RenderCommand>::make<RenderCommandDraw>(std::move(*this), _pipeline_snapshot._bindings->snippet()->makeDrawEvents(renderRequest));
}

sp<RenderCommand> DrawingContext::toBindCommand()
{
    DCHECK(_pipeline_snapshot._bindings, "DrawingContext cannot be converted to RenderCommand more than once");
    return sp<RenderCommand>::make<RenderCommandBind>(std::move(*this));
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
