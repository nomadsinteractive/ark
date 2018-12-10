#include "renderer/base/drawing_context.h"

#include "graphics/base/camera.h"

#include "renderer/base/shader_bindings.h"
#include "renderer/base/gl_snippet_delegate.h"

namespace ark {

DrawingContext::DrawingContext(const sp<ShaderBindings>& shaderBindings, const Camera::Snapshot& camera, const Buffer::Snapshot& arrayBuffer, const Buffer::Snapshot& indexBuffer)
    : _shader_bindings(shaderBindings), _camera(camera), _array_buffer(arrayBuffer), _index_buffer(indexBuffer), _count(indexBuffer.length<glindex_t>())
{
    DWARN(_shader_bindings->arrayBuffer().id() == arrayBuffer.id(), "GLShaderBinding's ArrayBuffer: %d, which is not the same as GLDrawingContext's ArrayBuffer snapshot: %d", _shader_bindings->arrayBuffer().id(), arrayBuffer.id());
}

void DrawingContext::preDraw(GraphicsContext& graphicsContext, const Shader& shader)
{
    _array_buffer.upload(graphicsContext);
    _index_buffer.upload(graphicsContext);
    DCHECK(_array_buffer.id(), "Invaild GL Array Buffer");
    DCHECK(_index_buffer.id(), "Invaild GL Index Buffer");

    for(const auto& iter : _instanced_array_snapshots)
    {
        iter.second.upload(graphicsContext);
        DCHECK(iter.second.id(), "Invaild GL Instanced Array Buffer: %d", iter.first);
    }

    _shader_bindings->snippet()->preDraw(graphicsContext, shader, *this);
}

void DrawingContext::postDraw(GraphicsContext& graphicsContext)
{
    _shader_bindings->snippet()->postDraw(graphicsContext);
}

}
