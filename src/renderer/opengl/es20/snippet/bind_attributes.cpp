#include "renderer/opengl/es20/snippet/bind_attributes.h"

#include "renderer/base/drawing_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/inf/resource.h"

#include "renderer/opengl/base/gl_pipeline.h"

namespace ark {
namespace gles20 {

void BindAttributes::preDraw(GraphicsContext& graphicsContext, const Shader& shader, const DrawingContext& context)
{
    glBindBuffer(GL_ARRAY_BUFFER, context._array_buffer.id());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, context._index_buffer.id());
    shader.pipeline()->bind(graphicsContext, context._shader_bindings);
}

}
}
