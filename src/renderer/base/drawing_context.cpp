#include "renderer/base/drawing_context.h"

#include "pipeline_descriptor.h"
#include "graphics/inf/render_command.h"

#include "renderer/base/pipeline_bindings.h"
#include "renderer/impl/draw_decorator/draw_decorator_composite.h"
#include "renderer/inf/draw_decorator.h"
#include "renderer/inf/pipeline.h"

namespace ark {

namespace {

class RenderCommandNoopWithDrawDecorator final : public RenderCommand {
public:
    RenderCommandNoopWithDrawDecorator(DrawingContext context, sp<DrawDecorator> drawDecorator)
        : _context(std::move(context)), _draw_decorator(std::move(drawDecorator)) {
    }

    void draw(GraphicsContext& graphicsContext) override {
        _draw_decorator->preDraw(graphicsContext, _context);
        _context._bindings->ensurePipeline(graphicsContext);
        _draw_decorator->postDraw(graphicsContext, _context);
    }

private:
    DrawingContext _context;
    sp<DrawDecorator> _draw_decorator;
};

class RenderCommandNoop final : public RenderCommand {
public:
    RenderCommandNoop(DrawingContext context)
        : _context(std::move(context)) {
    }

    void draw(GraphicsContext& graphicsContext) override {
        _context._bindings->ensurePipeline(graphicsContext);
    }

private:
    DrawingContext _context;
};

class RenderCommandDrawWithDrawDecorator final : public RenderCommand {
public:
    RenderCommandDrawWithDrawDecorator(DrawingContext context, sp<DrawDecorator> drawDecorator)
        : _context(std::move(context)), _draw_decorator(std::move(drawDecorator)) {
    }

    void draw(GraphicsContext& graphicsContext) override {
        DPROFILER_TRACE("DrawCommand");

        _draw_decorator->preDraw(graphicsContext, _context);
        _context.upload(graphicsContext);

        const sp<Pipeline>& pipeline = _context._bindings->ensurePipeline(graphicsContext);
        pipeline->draw(graphicsContext, _context);
        _draw_decorator->postDraw(graphicsContext, _context);
    }

private:
    DrawingContext _context;
    sp<DrawDecorator> _draw_decorator;
};

class RenderCommandDraw final : public RenderCommand {
public:
    RenderCommandDraw(DrawingContext context)
        : _context(std::move(context)) {
    }

    void draw(GraphicsContext& graphicsContext) override {
        DPROFILER_TRACE("DrawCommand");

        _context.upload(graphicsContext);

        const sp<Pipeline>& pipeline = _context._bindings->ensurePipeline(graphicsContext);
        pipeline->draw(graphicsContext, _context);
    }

private:
    DrawingContext _context;
};

}

DrawingContext::DrawingContext(sp<PipelineBindings> pipelineBindings, sp<RenderBufferSnapshot> bufferSnapshot, Buffer::Snapshot vertices, Buffer::Snapshot indices, const uint32_t drawCount, DrawingParams parameters, const sp<Traits>& attachments)
    : PipelineContext{std::move(pipelineBindings), std::move(bufferSnapshot)}, _vertices(std::move(vertices)), _indices(std::move(indices)), _draw_count(drawCount), _parameters(std::move(parameters)), _attachments(attachments ? attachments : _bindings->attachments())
{
}

sp<RenderCommand> DrawingContext::toRenderCommand(const RenderRequest& renderRequest, sp<DrawDecorator> drawDecorator)
{
    DCHECK(_bindings, "DrawingContext cannot be converted to RenderCommand more than once");
    if(const PipelineDescriptor::Configuration& conf = _bindings->pipelineDescriptor()->configuration(); conf._draw_decorator_factory)
        drawDecorator = DrawDecoratorComposite::compose(conf._draw_decorator_factory->makeDrawDecorator(renderRequest), std::move(drawDecorator));
    if(drawDecorator)
        return sp<RenderCommand>::make<RenderCommandDrawWithDrawDecorator>(std::move(*this), std::move(drawDecorator));
    return sp<RenderCommand>::make<RenderCommandDraw>(std::move(*this));
}

sp<RenderCommand> DrawingContext::toNoopCommand(const RenderRequest& renderRequest, sp<DrawDecorator> drawDecorator)
{
    DCHECK(_bindings, "DrawingContext cannot be converted to RenderCommand more than once");
    if(const PipelineDescriptor::Configuration& conf = _bindings->pipelineDescriptor()->configuration(); conf._draw_decorator_factory)
        drawDecorator = DrawDecoratorComposite::compose(conf._draw_decorator_factory->makeDrawDecorator(renderRequest), std::move(drawDecorator));
    if(drawDecorator)
        return sp<RenderCommand>::make<RenderCommandNoopWithDrawDecorator>(std::move(*this), std::move(drawDecorator));
    return sp<RenderCommand>::make<RenderCommandNoop>(std::move(*this));
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
