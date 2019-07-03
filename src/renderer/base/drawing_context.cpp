#include "renderer/base/drawing_context.h"

#include "core/base/object_pool.h"

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

DrawingContext::DrawingContext(const sp<Shader>& shader, const sp<ShaderBindings>& shaderBindings, std::vector<RenderLayer::UBOSnapshot> ubo)
    : _shader(shader), _shader_bindings(shaderBindings), _ubos(std::move(ubo))
{
}

DrawingContext::DrawingContext(const sp<Shader>& shader, const sp<ShaderBindings>& shaderBindings, std::vector<RenderLayer::UBOSnapshot> ubo, const Buffer::Snapshot& vertexBuffer, const Buffer::Snapshot& indexBuffer, int32_t instanceCount)
    : DrawingContext(shader, shaderBindings, std::move(ubo), vertexBuffer, indexBuffer, instanceCount, 0, indexBuffer.length<element_index_t>())
{
}

DrawingContext::DrawingContext(const sp<Shader>& shader, const sp<ShaderBindings>& shaderBindings, std::vector<RenderLayer::UBOSnapshot> ubo, const Buffer::Snapshot& vertexBuffer, const Buffer::Snapshot& indexBuffer, int32_t instanceCount, uint32_t start, uint32_t count)
    : _shader(shader), _shader_bindings(shaderBindings), _ubos(std::move(ubo)), _vertex_buffer(vertexBuffer), _index_buffer(indexBuffer), _parameters(instanceCount, start, count, true)
{
    DWARN(_shader_bindings->vertexBuffer().id() == vertexBuffer.id(), "ShaderBinding's VertexBuffer: %lld, which is not the same as DrawingContext's VertexBuffer snapshot: %lld", _shader_bindings->vertexBuffer().id(), vertexBuffer.id());
}

sp<RenderCommand> DrawingContext::toRenderCommand(ObjectPool& objectPool)
{
    DCHECK(_shader && _shader_bindings, "DrawingContext cannot be converted to RenderCommand more than once");
    if(_parameters._count)
        return objectPool.obtain<RenderCommandDraw>(std::move(*this));
    return objectPool.obtain<RenderCommandBind>(std::move(*this));
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

DrawingContext::Parameters::Parameters(int32_t instanceCount, uint32_t start, uint32_t count, bool cullFace)
    : _instance_count(instanceCount), _start(start), _count(count), _scissor(0, 0, -1.0f, -1.0f)
{
}

}
