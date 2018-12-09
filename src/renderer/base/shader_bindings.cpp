#include "renderer/base/shader_bindings.h"

#include "renderer/opengl/base/gl_pipeline.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/gl_snippet_delegate.h"
#include "renderer/base/shader.h"
#include "renderer/base/texture.h"

#include "renderer/impl/gl_snippet/gl_snippet_active_texture.h"

namespace ark {

ShaderBindings::ShaderBindings(GLResourceManager& resourceManager, const sp<Shader>& shader)
    : ShaderBindings(resourceManager, shader, resourceManager.makeDynamicArrayBuffer())
{
}

ShaderBindings::ShaderBindings(GLResourceManager& resourceManager, const sp<Shader>& shader, const Buffer& arrayBuffer)
    : _shader(shader), _snippet(sp<GLSnippetDelegate>::make(shader)), _attributes(shader->input()), _array_buffer(arrayBuffer),
      _pipeline_input(_shader->input()), _instanced_arrays(_pipeline_input->makeInstancedArrays(resourceManager))
{
}

const sp<Shader>& ShaderBindings::shader() const
{
    return _shader;
}

const sp<GLSnippetDelegate>& ShaderBindings::snippet() const
{
    return _snippet;
}

const sp<PipelineInput>& ShaderBindings::pipelineInput() const
{
    return _pipeline_input;
}

const Buffer& ShaderBindings::arrayBuffer() const
{
    return _array_buffer;
}

const std::vector<std::pair<uint32_t, Buffer>>& ShaderBindings::instancedArrays() const
{
    return _instanced_arrays;
}

const ShaderBindings::Attributes& ShaderBindings::attributes() const
{
    return _attributes;
}

void ShaderBindings::bindGLTexture(const sp<Texture>& texture, uint32_t name) const
{
    _snippet->link<GLSnippetActiveTexture>(texture, name);
}

void ShaderBindings::bindGLTexture(const sp<RenderResource>& texture, uint32_t target, uint32_t name) const
{
    _snippet->link<GLSnippetActiveTexture>(texture, target, name);
}

std::map<uint32_t, Buffer::Builder> ShaderBindings::makeInstancedBufferBuilders(const sp<MemoryPool>& memoryPool, const sp<ObjectPool>& objectPool, size_t instanceCount) const
{
    std::map<uint32_t, Buffer::Builder> builders;
    for(const std::pair<uint32_t, Buffer>& i : _instanced_arrays)
    {
        const PipelineInput::Stream& stream = _pipeline_input->getStream(i.first);
        builders.insert(std::make_pair(i.first, Buffer::Builder(memoryPool, objectPool, stream.stride(), instanceCount / i.first)));
    }
    return builders;
}

ShaderBindings::Attributes::Attributes(const PipelineInput& input)
{
    _offsets[ATTRIBUTE_NAME_TEX_COORDINATE] = input.getAttributeOffset("TexCoordinate");
    _offsets[ATTRIBUTE_NAME_NORMAL] = input.getAttributeOffset("Normal");
    _offsets[ATTRIBUTE_NAME_TANGENT] = input.getAttributeOffset("Tangent");
    _offsets[ATTRIBUTE_NAME_BITANGENT] = input.getAttributeOffset("Bitangent");
}

}
