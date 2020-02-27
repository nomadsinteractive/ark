#include "renderer/base/drawing_context.h"

#include "graphics/base/camera.h"
#include "graphics/inf/render_command.h"

#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/snippet_delegate.h"
#include "renderer/inf/pipeline.h"

namespace ark {

class DrawingContext::RenderCommandBind : public RenderCommand {
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

class DrawingContext::RenderCommandDraw : public RenderCommand {
public:
    RenderCommandDraw(DrawingContext context)
        : _context(std::move(context)) {
    }

    virtual void draw(GraphicsContext& graphicsContext) override {
        _context.upload(graphicsContext);

        const sp<Pipeline> pipeline = _context._shader_bindings->getPipeline(graphicsContext);
        _context.preDraw(graphicsContext);
        pipeline->bind(graphicsContext, _context);
        pipeline->draw(graphicsContext, _context);
        _context.postDraw(graphicsContext);
    }

private:
    DrawingContext _context;

};

DrawingContext::DrawingContext(const sp<ShaderBindings>& shaderBindings, const sp<ByType>& attachments, std::vector<RenderLayer::UBOSnapshot> ubo)
    : _shader_bindings(shaderBindings), _attachments(attachments), _ubos(std::move(ubo))
{
}

DrawingContext::DrawingContext(const sp<ShaderBindings>& shaderBindings, const sp<ByType>& attachments, std::vector<RenderLayer::UBOSnapshot> ubo, Buffer::Snapshot vertexBuffer, Buffer::Snapshot indexBuffer, int32_t instanceCount)
    : DrawingContext(shaderBindings, attachments, std::move(ubo), std::move(vertexBuffer), std::move(indexBuffer), instanceCount, 0, indexBuffer.length<element_index_t>())
{
}

DrawingContext::DrawingContext(const sp<ShaderBindings>& shaderBindings, const sp<ByType>& attachments, std::vector<RenderLayer::UBOSnapshot> ubo, Buffer::Snapshot vertexBuffer, Buffer::Snapshot indexBuffer, int32_t instanceCount, uint32_t start, uint32_t count)
    : _shader_bindings(shaderBindings), _attachments(attachments), _ubos(std::move(ubo)), _vertex_buffer(std::move(vertexBuffer)), _index_buffer(std::move(indexBuffer)), _parameters(instanceCount, start, count)
{
}

sp<RenderCommand> DrawingContext::toRenderCommand()
{
    DCHECK(_shader_bindings, "DrawingContext cannot be converted to RenderCommand more than once");
    if(_parameters._count)
        return sp<RenderCommandDraw>::make(std::move(*this));
    return sp<RenderCommandBind>::make(std::move(*this));
}

void DrawingContext::upload(GraphicsContext& graphicsContext)
{
    _vertex_buffer.upload(graphicsContext);
    _index_buffer.upload(graphicsContext);
    DCHECK(_vertex_buffer.id(), "Invaild VertexBuffer");
    DCHECK(_index_buffer.id(), "Invaild IndexBuffer");

    for(const auto& i : _instanced_array_snapshots)
    {
        i.second.upload(graphicsContext);
        DCHECK(i.second.id(), "Invaild Instanced Array Buffer: %d", i.first);
    }
}

void DrawingContext::preDraw(GraphicsContext& graphicsContext)
{
    _shader_bindings->snippet()->preDraw(graphicsContext, *this);
}

void DrawingContext::postDraw(GraphicsContext& graphicsContext)
{
    _shader_bindings->snippet()->postDraw(graphicsContext);
}

DrawingContext::Parameters::Parameters()
    : _instance_count(0), _start(0), _count(0)
{
}

DrawingContext::Parameters::Parameters(int32_t instanceCount, uint32_t start, uint32_t count)
    : _instance_count(instanceCount), _start(start), _count(count), _scissor(0, 0, -1.0f, -1.0f)
{
}

}
