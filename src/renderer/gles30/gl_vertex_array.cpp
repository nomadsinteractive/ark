#include "renderer/gles30/gl_vertex_array.h"

#include "core/util/log.h"

#include "renderer/base/gl_shader_bindings.h"

#include "platform/gl/gl.h"

namespace ark {
namespace gles30 {

GLVertexArray::GLVertexArray(const sp<GLShaderBindings>& shaderBindings, const GLShader& shader)
    : _id(0), _shader_bindings(shaderBindings), _shader(shader)
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
    _shader_bindings->arrayBuffer().prepare(graphicsContext);
    glBindBuffer(GL_ARRAY_BUFFER, _shader_bindings->arrayBuffer().id());
    const sp<GLProgram>& program = _shader.getGLProgram(graphicsContext);
    _shader_bindings->bindArrayBuffers(graphicsContext, program);
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
