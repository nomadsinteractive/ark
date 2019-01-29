#include "renderer/base/drawing_context.h"

#include "core/base/object_pool.h"

#include "graphics/base/camera.h"
#include "graphics/inf/render_command.h"

#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/snippet_delegate.h"
#include "renderer/inf/pipeline.h"

namespace ark {

class DrawingContext::RenderCommandImpl : public RenderCommand {
public:
    RenderCommandImpl(DrawingContext context)
        : _context(std::move(context)) {
    }

    virtual void draw(GraphicsContext& graphicsContext) override {
        _context.upload(graphicsContext);

        const sp<Pipeline> pipeline = _context._shader->getPipeline(graphicsContext, _context._shader_bindings);

        _context.preDraw(graphicsContext);
        pipeline->draw(graphicsContext, _context);
        _context.postDraw(graphicsContext);
    }

private:
    DrawingContext _context;

};

DrawingContext::DrawingContext(const sp<Shader>& shader, const sp<ShaderBindings>& shaderBindings, std::vector<Layer::UBOSnapshot> ubo, const Buffer::Snapshot& arrayBuffer, const Buffer::Snapshot& indexBuffer, int32_t instanceCount)
    : _shader(shader), _shader_bindings(shaderBindings), _ubos(std::move(ubo)), _array_buffer(arrayBuffer), _index_buffer(indexBuffer), _count(indexBuffer.length<glindex_t>()), _instance_count(instanceCount)
{
    DWARN(_shader_bindings->vertexBuffer().id() == arrayBuffer.id(), "ShaderBinding's VertexBuffer: %d, which is not the same as DrawingContext's ArrayBuffer snapshot: %d", _shader_bindings->vertexBuffer().id(), arrayBuffer.id());
}

sp<RenderCommand> DrawingContext::toRenderCommand(ObjectPool& objectPool)
{
    DCHECK(_shader && _shader_bindings, "DrawingContext cannot be converted to RenderCommand more than once");
    return objectPool.obtain<RenderCommandImpl>(std::move(*this));
}

void DrawingContext::upload(GraphicsContext& graphicsContext)
{
    _array_buffer.upload(graphicsContext);
    _index_buffer.upload(graphicsContext);
    DCHECK(_array_buffer.id(), "Invaild VertexBuffer");
    DCHECK(_index_buffer.id(), "Invaild IndexBuffer");

    for(const auto& iter : _instanced_array_snapshots)
    {
        iter.second.upload(graphicsContext);
        DCHECK(iter.second.id(), "Invaild GL Instanced Array Buffer: %d", iter.first);
    }
}

void DrawingContext::preDraw(GraphicsContext& graphicsContext)
{
    _shader_bindings->snippet()->preDraw(graphicsContext, _shader, *this);
}

void DrawingContext::postDraw(GraphicsContext& graphicsContext)
{
    _shader_bindings->snippet()->postDraw(graphicsContext);
}

}
