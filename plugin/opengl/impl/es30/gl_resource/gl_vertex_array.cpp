#include "opengl/impl/es30/gl_resource/gl_vertex_array.h"

#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/pipeline_bindings.h"

#include "platform/gl/gl.h"

namespace ark::plugin::opengl {

GLVertexArray::GLVertexArray(sp<GLPipeline> pipeline, sp<Buffer::Delegate> vertices, const PipelineBindings& pipelineBindings)
    : _pipeline(std::move(pipeline)), _vertex(std::move(vertices)), _pipeline_descriptor(pipelineBindings.pipelineDescriptor()), _streams(pipelineBindings.streams()), _id(0)
{
}

uint64_t GLVertexArray::id()
{
    return _id;
}

void GLVertexArray::upload(GraphicsContext& graphicsContext)
{
    glGenVertexArrays(1, &_id);
    glBindVertexArray(_id);
    _vertex->upload(graphicsContext);
    glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(_vertex->id()));
    _pipeline->bindBuffer(graphicsContext, _pipeline_descriptor->layout(), _streams);
    glBindVertexArray(0);
}

ResourceRecycleFunc GLVertexArray::recycle()
{
    uint32_t id = _id;
    _id = 0;
    return [id](GraphicsContext&) {
        glDeleteVertexArrays(1, &id);
    };
}

}
