#include "renderer/opengl/es30/gl_resource/gl_vertex_array.h"

#include "core/util/log.h"

#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/inf/pipeline_factory.h"

#include "platform/gl/gl.h"

namespace ark {
namespace gles30 {

GLVertexArray::GLVertexArray(sp<opengl::GLPipeline> pipeline, Buffer vertices, const ShaderBindings& shaderBindings)
    : _pipeline(std::move(pipeline)), _pipeline_bindings(shaderBindings.pipelineBindings()), _vertex(std::move(vertices)), _divisors(shaderBindings.divisors()), _id(0)
{
}

uint64_t GLVertexArray::id()
{
    return _id;
}

void GLVertexArray::upload(GraphicsContext& graphicsContext, const sp<Uploader>& /*uploader*/)
{
    glGenVertexArrays(1, &_id);
    glBindVertexArray(_id);
    _vertex.upload(graphicsContext);
    glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(_vertex.id()));
    _pipeline->bindBuffer(graphicsContext, _pipeline_bindings->input(), _divisors);
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
