#include "renderer/gles30/impl/gl_snippet/bind_vertex_array.h"

#include "renderer/base/gl_shader.h"
#include "renderer/base/gl_snippet_context.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/graphics_context.h"

#include "renderer/gles30/gl_vertex_array.h"

namespace ark {
namespace gles30 {

BindVertexArray::BindVertexArray(const sp<GLResource>& vertexArray)
    : _vertex_array(vertexArray)
{
}

void BindVertexArray::preDraw(GraphicsContext& graphicsContext, const GLShader& /*shader*/, const GLSnippetContext& context)
{
    context._array_buffer.prepare(graphicsContext);
    context._index_buffer.prepare(graphicsContext);
    DCHECK(context._array_buffer.id() && context._index_buffer.id(), "Invaild GLBuffer");

    glBindVertexArray(_vertex_array->id());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, context._index_buffer.id());
}

}
}
