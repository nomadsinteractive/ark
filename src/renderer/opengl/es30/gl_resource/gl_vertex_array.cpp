#include "renderer/opengl/es30/gl_resource/gl_vertex_array.h"

#include "core/util/log.h"

#include "renderer/base/shader_bindings.h"
#include "renderer/opengl/base/gl_pipeline.h"

#include "platform/gl/gl.h"

namespace ark {
namespace gles30 {

GLVertexArray::GLVertexArray(const sp<ShaderBindings>& shaderBindings, const Shader& shader)
    : _id(0), _shader_bindings(shaderBindings), _shader(shader)
{
}

uint32_t GLVertexArray::id()
{
    return _id;
}

void GLVertexArray::upload(GraphicsContext& graphicsContext)
{
    glGenVertexArrays(1, &_id);
    glBindVertexArray(_id);
    _shader_bindings->arrayBuffer().upload(graphicsContext);
    glBindBuffer(GL_ARRAY_BUFFER, _shader_bindings->arrayBuffer().id());
    const sp<Pipeline> pipeline = _shader.getPipeline(graphicsContext, _shader_bindings);
    pipeline->bind(graphicsContext, _shader_bindings);
    glBindVertexArray(0);
    LOGD("id = %d", _id);
}

Resource::RecycleFunc GLVertexArray::recycle()
{
    uint32_t id = _id;
    _id = 0;
    return [id](GraphicsContext&) {
        LOGD("Deleting GLVertexArray[%d]", id);
        glDeleteVertexArrays(1, &id);
    };
}

}
}
