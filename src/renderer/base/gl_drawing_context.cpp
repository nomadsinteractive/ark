#include "renderer/base/gl_drawing_context.h"

#include "renderer/inf/gl_snippet.h"

namespace ark {

GLDrawingContext::GLDrawingContext(const sp<GLSnippet>& snippet, const GLBuffer::Snapshot& arrayBuffer, const GLBuffer& indexBuffer, GLenum mode)
    : _snippet(snippet), _array_buffer(arrayBuffer), _index_buffer(indexBuffer), _mode(mode)
{
}

void GLDrawingContext::preDraw(GraphicsContext& graphicsContext, const GLShader& shader)
{
    _snippet->preDraw(graphicsContext, shader, *this);
}

void GLDrawingContext::postDraw(GraphicsContext& graphicsContext)
{
    _snippet->postDraw(graphicsContext);
}

}
