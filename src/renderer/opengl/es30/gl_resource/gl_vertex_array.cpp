#include "renderer/opengl/es30/gl_resource/gl_vertex_array.h"

#include "core/util/log.h"

#include "renderer/base/shader_bindings.h"
#include "renderer/inf/pipeline_factory.h"

#include "renderer/opengl/base/gl_pipeline.h"

#include "platform/gl/gl.h"

namespace ark {
namespace gles30 {

GLVertexArray::GLVertexArray(const sp<PipelineFactory>& pipelineFactory, const sp<ShaderBindings>& shaderBindings)
    : _id(0), _pipeline_factory(pipelineFactory), _shader_bindings(shaderBindings)
{
}

uint64_t GLVertexArray::id()
{
    return _id;
}

void GLVertexArray::upload(GraphicsContext& graphicsContext, const sp<Uploader>& uploader)
{
    const sp<ShaderBindings> bindings = _shader_bindings.ensure();
    glGenVertexArrays(1, &_id);
    glBindVertexArray(_id);
    bindings->vertexBuffer().upload(graphicsContext);
    glBindBuffer(GL_ARRAY_BUFFER, bindings->vertexBuffer().id());
    const sp<opengl::GLPipeline> pipeline = _pipeline_factory->buildPipeline(graphicsContext, bindings);
    pipeline->bindBuffer(graphicsContext, bindings);
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
