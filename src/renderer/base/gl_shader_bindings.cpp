#include "renderer/base/gl_shader_bindings.h"

#include "renderer/base/gl_program.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/gl_shader.h"
#include "renderer/base/gl_shader_source.h"
#include "renderer/base/gl_snippet_delegate.h"
#include "renderer/impl/gl_snippet/gl_snippet_active_texture.h"

namespace ark {

GLShaderBindings::GLShaderBindings(GLResourceManager& resourceManager, const sp<GLShader>& shader)
    : GLShaderBindings(resourceManager, shader, resourceManager.makeDynamicArrayBuffer())
{
}

GLShaderBindings::GLShaderBindings(GLResourceManager& resourceManager, const sp<GLShader>& shader, const GLBuffer& arrayBuffer)
    : _shader(shader), _snippet(sp<GLSnippetDelegate>::make(shader)), _attributes(shader->source()->input()), _array_buffer(arrayBuffer),
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

const std::vector<std::pair<uint32_t, GLBuffer>>& GLShaderBindings::instancedArrays() const
{
    return _instanced_arrays;
}

const GLShaderBindings::Attributes& GLShaderBindings::attributes() const
{
    return _attributes;
}

void GLShaderBindings::bindArrayBuffers(GraphicsContext& graphicsContext, GLProgram& program) const
{
    DCHECK(program.id(), "GLProgram unprepared");
    _shader_input->bind(graphicsContext, program, 0);
    for(const auto& i : _instanced_arrays)
    {
        glBindBuffer(GL_ARRAY_BUFFER, i.second.id());
        _shader_input->bind(graphicsContext, program, i.first);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void GLShaderBindings::bindGLTexture(const sp<GLTexture>& texture, uint32_t name) const
{
    _snippet->link<GLSnippetActiveTexture>(texture, name);
}

void GLShaderBindings::bindGLTexture(const sp<GLResource>& texture, uint32_t target, uint32_t name) const
{
    _snippet->link<GLSnippetActiveTexture>(texture, target, name);
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
    _offsets[ATTRIBUTE_NAME_BITANGENT] = input.getAttributeOffset("Bitangent");
}

}
