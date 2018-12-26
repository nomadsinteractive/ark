#include "renderer/base/shader_bindings.h"

#include "renderer/base/resource_manager.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/snippet_delegate.h"
#include "renderer/base/shader.h"
#include "renderer/base/texture.h"

namespace ark {

ShaderBindings::ShaderBindings(RenderController& renderController, const sp<Shader>& shader)
    : ShaderBindings(renderController, shader, renderController.makeVertexBuffer())
{
}

ShaderBindings::ShaderBindings(RenderController& renderController, const sp<Shader>& shader, const Buffer& arrayBuffer)
    : _attributes(shader->input()), _array_buffer(arrayBuffer), _pipeline_layout(shader->pipelineLayout()), _pipeline_input(shader->input()),
      _instanced_arrays(_pipeline_input->makeInstancedArrays(renderController)), _render_mode(RenderModel::RENDER_MODE_NONE)
{
    _samplers.resize(_pipeline_input->samplerCount());
}

const sp<Snippet>& ShaderBindings::snippet() const
{
    return _pipeline_layout->snippet();
}

const sp<PipelineLayout>& ShaderBindings::pipelineLayout() const
{
    return _pipeline_layout;
}

const sp<PipelineInput>& ShaderBindings::pipelineInput() const
{
    return _pipeline_input;
}

const std::vector<sp<Texture>>& ShaderBindings::samplers() const
{
    return _samplers;
}

bool ShaderBindings::isDrawInstanced() const
{
    return _instanced_arrays.size() > 0;
}

RenderModel::Mode ShaderBindings::renderMode() const
{
    return _render_mode;
}

void ShaderBindings::setRenderModel(RenderModel& renderModel)
{
    renderModel.initialize(*this);
    _render_mode = renderModel.mode();
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

void ShaderBindings::bindSampler(const sp<Texture>& texture, uint32_t name)
{
    DCHECK(_samplers.size() > name, "Illegal sampler binding position: %d, sampler count: %d", name, _samplers.size());
    _samplers[name] = texture;
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
