#include "renderer/base/shader_bindings.h"

#include "core/base/memory_pool.h"
#include "core/inf/flatable.h"

#include "renderer/opengl/base/gl_pipeline.h"
#include "renderer/base/resource_manager.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/snippet_delegate.h"
#include "renderer/base/shader.h"
#include "renderer/base/texture.h"

#include "renderer/impl/snippet/snippet_active_texture.h"

namespace ark {

ShaderBindings::ShaderBindings(RenderController& renderController, const sp<Shader>& shader)
    : ShaderBindings(renderController, shader, renderController.makeVertexBuffer())
{
}

ShaderBindings::ShaderBindings(RenderController& renderController, const sp<Shader>& shader, const Buffer& arrayBuffer)
    : _shader(shader), _snippet(sp<SnippetDelegate>::make(shader)), _attributes(shader->input()), _ubo_manifest(shader->input()), _array_buffer(arrayBuffer),
      _pipeline_input(_shader->input()), _instanced_arrays(_pipeline_input->makeInstancedArrays(renderController))
{
}

const sp<Shader>& ShaderBindings::shader() const
{
    return _shader;
}

const sp<SnippetDelegate>& ShaderBindings::snippet() const
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

Layer::UBO ShaderBindings::snapshot(MemoryPool& memoryPool, const Camera& camera) const
{
    Layer::UBO ubo(camera.snapshot());
    bool dirty = _ubo_manifest.snapshot(_pipeline_input);
    if(dirty)
    {
        ubo._dirty_flags = memoryPool.allocate(_ubo_manifest._dirty_flags->size());
        memcpy(ubo._dirty_flags->buf(), _ubo_manifest._dirty_flags->buf(), _ubo_manifest._dirty_flags->size());
        ubo._buffer = memoryPool.allocate(_ubo_manifest._buffer->size());
        memcpy(ubo._buffer->buf(), _ubo_manifest._buffer->buf(), _ubo_manifest._buffer->size());
    }
    return ubo;
}

const ShaderBindings::Attributes& ShaderBindings::attributes() const
{
    return _attributes;
}

const ShaderBindings::UBOManifest& ShaderBindings::uboManifest() const
{
    return _ubo_manifest;
}

void ShaderBindings::bindGLTexture(const sp<Texture>& texture, uint32_t name) const
{
    _snippet->link<SnippetActiveTexture>(texture, name);
}

void ShaderBindings::bindGLTexture(const sp<Resource>& texture, Texture::Type type, uint32_t name) const
{
    _snippet->link<SnippetActiveTexture>(texture, type, name);
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

ShaderBindings::UBOManifest::UBOManifest(const PipelineInput& input)
    : _dirty_flags(sp<DynamicArray<uint8_t>>::make(input.uniforms().size()))
{
    size_t size = 0;
    const std::vector<Uniform>& uniforms = input.uniforms();
    for(const auto& i : uniforms)
    {
        size_t s = i.flatable()->size();
        _slots.push_back(std::make_pair(size, s));
        size += s;
        DCHECK(size % 4 == 0, "Uniform aligment error, offset: %d", size);
    }
    _buffer = sp<DynamicArray<uint8_t>>::make(size);
    memset(_buffer->buf(), 0, _buffer->size());

    snapshot(input);
}

bool ShaderBindings::UBOManifest::snapshot(const PipelineInput& input) const
{
    size_t offset = 0;
    const std::vector<Uniform>& uniforms = input.uniforms();
    uint8_t* dirtyFlags = _dirty_flags->buf();
    bool dirty = false;
    for(size_t i = 0; i < uniforms.size(); ++i)
    {
        const Uniform& uniform = uniforms.at(i);
        const sp<Flatable>& flatable = uniform.flatable();
        dirtyFlags[i] = static_cast<uint8_t>(uniform.notifier()->hasChanged());
        dirty = dirty || dirtyFlags[i];
        if(dirtyFlags[i])
            flatable->flat(_buffer->buf() + offset);
        offset += flatable->size();
    }
    return dirty;
}

}
