#include "renderer/gles30/gl_vertex_array.h"

#include "core/util/log.h"

#include "renderer/base/gl_program.h"
#include "renderer/base/gl_shader.h"
#include "renderer/base/graphics_context.h"

#include "platform/gl/gl.h"

namespace ark {
namespace gles30 {

GLVertexArray::GLVertexArray(const GLShader& shader, const GLBuffer& arrayBuffer)
    : _id(0), _shader(shader), _array_buffer(arrayBuffer)
{
}

uint32_t GLVertexArray::id()
{
    return _id;
}

void GLVertexArray::prepare(GraphicsContext& graphicsContext)
{
    glGenVertexArrays(1, &_id);
    glBindVertexArray(_id);
    _array_buffer.prepare(graphicsContext);
    glBindBuffer(GL_ARRAY_BUFFER, _array_buffer.id());
    const sp<GLProgram> program = graphicsContext.getGLProgram(_shader);
    _shader.bindAttributes(graphicsContext, program);
    glBindVertexArray(0);
    LOGD("id = %d", _id);
}

void GLVertexArray::recycle(GraphicsContext& /*graphicsContext*/)
{
    LOGD("Deleting GLVertexArray[%d]", _id);
    glDeleteVertexArrays(1, &_id);
    _id = 0;
}

}
}
