#include "renderer/gles20/impl/gl_snippet/bind_attributes.h"

#include "renderer/base/gl_drawing_context.h"
#include "renderer/base/gl_shader.h"
#include "renderer/inf/gl_resource.h"

namespace ark {
namespace gles20 {

void BindAttributes::preDraw(GraphicsContext& graphicsContext, const GLShader& shader, const GLDrawingContext& context)
{
    context._array_buffer.prepare(graphicsContext);
    context._index_buffer.prepare(graphicsContext);
    DCHECK(context._array_buffer.id() && context._index_buffer.id(), "Invaild GLBuffer");

    glBindBuffer(GL_ARRAY_BUFFER, context._array_buffer.id());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, context._index_buffer.id());
    shader.bindAttributes(graphicsContext);
}

}
}
