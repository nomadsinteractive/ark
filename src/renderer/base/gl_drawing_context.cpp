#include "renderer/base/gl_drawing_context.h"

#include "renderer/base/gl_shader_bindings.h"
#include "renderer/base/gl_snippet_delegate.h"

namespace ark {

GLDrawingContext::GLDrawingContext(const sp<GLShaderBindings>& shaderBindings, const GLBuffer::Snapshot& arrayBuffer, const GLBuffer::Snapshot& indexBuffer, GLenum mode)
    : _shader_bindings(shaderBindings), _array_buffer(arrayBuffer), _index_buffer(indexBuffer), _mode(mode)
{
}

void GLDrawingContext::preDraw(GraphicsContext& graphicsContext, const GLShader& shader)
{
    _array_buffer.prepare(graphicsContext);
    _index_buffer.prepare(graphicsContext);
    DCHECK(_array_buffer.id(), "Invaild GL Array Buffer");
    DCHECK(_index_buffer.id(), "Invaild GL Index Buffer");

    for(const auto& iter : _instanced_array_buffers)
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
