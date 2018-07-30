#include "renderer/base/gl_shader_bindings.h"

#include "renderer/base/gl_program.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/gl_shader.h"
#include "renderer/base/gl_shader_source.h"
#include "renderer/base/gl_snippet_delegate.h"

namespace ark {

GLShaderBindings::GLShaderBindings(GLResourceManager& resourceManager, const sp<GLShader>& shader)
    : _shader(shader), _snippet(sp<GLSnippetDelegate>::make(shader)), _attributes(shader->source()->input()), _array_buffer(resourceManager.makeDynamicArrayBuffer()),
      _shader_input(_shader->source()->input()), _instanced_arrays(_shader_input->makeInstancedArrays(resourceManager))
{
}

const sp<GLShader>& GLShaderBindings::shader() const
{
    return _shader;
}

const sp<GLSnippetDelegate>& GLShaderBindings::snippet() const
{
    return _snippet;
}

const GLBuffer& GLShaderBindings::arrayBuffer() const
{
    return _array_buffer;
}

const GLShaderBindings::Attributes&GLShaderBindings::attributes() const
{
    return _attributes;
}

void GLShaderBindings::bindArrayBuffers(GraphicsContext& graphicsContext, GLProgram& program) const
{
    DCHECK(program.id(), "GLProgram unprepared");
    _shader_input->bind(graphicsContext, program, 0);
    for(const auto iter : _instanced_arrays)
    {
        glBindBuffer(GL_ARRAY_BUFFER, iter.second.id());
        _shader_input->bind(graphicsContext, program, iter.first);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

std::map<uint32_t, GLBuffer::Builder> GLShaderBindings::makeInstancedBufferBuilders(const sp<MemoryPool>& memoryPool, const sp<ObjectPool>& objectPool, size_t instanceCount) const
{
    std::map<uint32_t, GLBuffer::Builder> builders;
    for(const std::pair<uint32_t, GLBuffer>& i : _instanced_arrays)
    {
        const GLShaderInput::Stream& stream = _shader_input->getStream(i.first);
        builders.insert(std::make_pair(i.first, GLBuffer::Builder(memoryPool, objectPool, stream.stride(), instanceCount / i.first)));
    }
    return builders;
}

GLShaderBindings::Attributes::Attributes(const GLShaderInput& input)
{
    _offsets[ATTRIBUTE_NAME_TEX_COORDINATE] = input.getAttributeOffset("TexCoordinate");
    _offsets[ATTRIBUTE_NAME_NORMAL] = input.getAttributeOffset("Normal");
    _offsets[ATTRIBUTE_NAME_TANGENT] = input.getAttributeOffset("Tangent");
}

}
