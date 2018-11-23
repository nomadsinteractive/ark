#include "renderer/base/gl_drawing_context.h"

#include "graphics/base/camera.h"

#include "renderer/base/gl_shader_bindings.h"
#include "renderer/base/gl_snippet_delegate.h"

namespace ark {

GLDrawingContext::GLDrawingContext(const sp<GLShaderBindings>& shaderBindings, const Camera::Snapshot& camera, const GLBuffer::Snapshot& arrayBuffer, const GLBuffer::Snapshot& indexBuffer, GLenum mode)
    : _shader_bindings(shaderBindings), _camera(camera), _array_buffer(arrayBuffer), _index_buffer(indexBuffer), _mode(mode), _count(indexBuffer.length<glindex_t>())
{
    DWARN(_shader_bindings->arrayBuffer().id() == arrayBuffer.id(), "GLShaderBinding's ArrayBuffer: %d, which is not the same as GLDrawingContext's ArrayBuffer snapshot: %d", _shader_bindings->arrayBuffer().id(), arrayBuffer.id());
}

void GLDrawingContext::preDraw(GraphicsContext& graphicsContext, const GLPipeline& shader)
{
    _array_buffer.prepare(graphicsContext);
    _index_buffer.prepare(graphicsContext);
    DCHECK(_array_buffer.id(), "Invaild GL Array Buffer");
    DCHECK(_index_buffer.id(), "Invaild GL Index Buffer");

    for(const auto& iter : _instanced_array_snapshots)
    {
        iter.second.prepare(graphicsContext);
        DCHECK(iter.second.id(), "Invaild GL Instanced Array Buffer: %d", iter.first);
    }

    _shader_bindings->snippet()->preDraw(graphicsContext, shader, *this);
}

void GLDrawingContext::postDraw(GraphicsContext& graphicsContext)
{
    _shader_bindings->snippet()->postDraw(graphicsContext);
}

}
