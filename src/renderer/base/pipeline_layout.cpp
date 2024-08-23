#include "renderer/base/pipeline_layout.h"

#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/shader_preprocessor.h"
#include "renderer/base/snippet_delegate.h"
#include "renderer/impl/snippet/snippet_linked_chain.h"
#include "renderer/inf/snippet.h"

namespace ark {

PipelineLayout::PipelineLayout(sp<PipelineBuildingContext> buildingContext)
    : _building_context(std::move(buildingContext)), _input(_building_context->_input), _color_attachment_count(0), _definitions(_building_context->toDefinitions())
{
}

void PipelineLayout::addSnippet(sp<Snippet> snippet)
{
    DASSERT(snippet);
    _snippet = _snippet ? sp<Snippet>::make<SnippetLinkedChain>(_snippet, std::move(snippet)) : std::move(snippet);
}

void PipelineLayout::preCompile(GraphicsContext& graphicsContext)
{
    if(_building_context)
    {
        _snippet->preCompile(graphicsContext, _building_context, *this);

        for(ShaderPreprocessor* preprocessor : _building_context->stages())
            _preprocessed_stages[preprocessor->_shader_stage] = preprocessor->preprocess();

        _building_context = nullptr;
    }
}

const sp<PipelineInput>& PipelineLayout::input() const
{
    return _input;
}

std::map<Enum::ShaderStageBit, String> PipelineLayout::getPreprocessedShaders(const RenderEngineContext& renderEngineContext) const
{
    std::map<Enum::ShaderStageBit, String> shaders;
    const std::map<String, String>& definitions = _definitions;

    for(const auto& [i, j] : _preprocessed_stages)
        shaders[i] = j.toSourceCode(renderEngineContext, definitions);

    return shaders;
}

const std::vector<PipelineInput::BindingSet>& PipelineLayout::samplers() const
{
    return _samplers;
}

const std::vector<PipelineInput::BindingSet>& PipelineLayout::images() const
{
    return _images;
}

size_t PipelineLayout::colorAttachmentCount() const
{
    return _color_attachment_count;
}

void PipelineLayout::initialize()
{
    _snippet = sp<Snippet>::make<SnippetDelegate>(_snippet);
    _snippet->preInitialize(_building_context);
    _building_context->initialize(*this);

    if(const ShaderPreprocessor* fragment = _building_context->tryGetRenderStage(Enum::SHADER_STAGE_BIT_FRAGMENT))
        _color_attachment_count = fragment->_main_block->outArgumentCount() + (fragment->_main_block->hasReturnValue() ? 1 : 0);

    _input->initialize(_building_context);
}

std::vector<std::pair<sp<Texture>, PipelineInput::BindingSet>> PipelineLayout::makeBindingSamplers() const
{
    DASSERT(_building_context);
    const PipelineInput& pipelineInput = _input;
    const Table<String, sp<Texture>>& predefinedSamplers = _building_context->_samplers;
    CHECK_WARN(pipelineInput.samplerCount() >= predefinedSamplers.size(), "Predefined samplers(%d) is more than samplers(%d) in PipelineLayout", predefinedSamplers.size(), _input->samplerCount());

    std::vector<std::pair<sp<Texture>, PipelineInput::BindingSet>> samplers;
    for(size_t i = 0; i < _samplers.size(); ++i)
    {
        const String& name = pipelineInput.samplerNames().at(i);
        const auto iter = predefinedSamplers.find(name);
        samplers.emplace_back(iter != predefinedSamplers.end() ? iter->second : (i < predefinedSamplers.size() ? predefinedSamplers.values().at(i) : nullptr), _samplers.at(i));
    }

    return samplers;
}

std::vector<std::pair<sp<Texture>, PipelineInput::BindingSet>> PipelineLayout::makeBindingImages() const
{
    DASSERT(_building_context);
    DASSERT(_building_context->_images.size() == _images.size());

    std::vector<std::pair<sp<Texture>, PipelineInput::BindingSet>> bindingImages;
    for(size_t i = 0; i < _images.size(); ++i)
        bindingImages.emplace_back(_building_context->_images.values().at(i), _images.at(i));
    return bindingImages;
}

const sp<Snippet>& PipelineLayout::snippet() const
{
    return _snippet;
}

}
