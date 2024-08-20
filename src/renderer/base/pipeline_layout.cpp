#include "renderer/base/pipeline_layout.h"

#include "core/inf/uploader.h"

#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/shader_preprocessor.h"
#include "renderer/inf/snippet.h"

namespace ark {

PipelineLayout::PipelineLayout(sp<PipelineBuildingContext> buildingContext, sp<Snippet> snippet)
    : _building_context(std::move(buildingContext)), _input(_building_context->_input), _snippet(std::move(snippet)),
      _color_attachment_count(0), _definitions(_building_context->toDefinitions())
{
    initialize();
}

void PipelineLayout::preCompile(GraphicsContext& graphicsContext)
{
    if(_building_context)
    {
        _snippet->preCompile(graphicsContext, _building_context, *this);

        for(const auto& [stage, preprocessor] : _building_context->stages())
            _preprocessed_stages[stage] = preprocessor->preprocess();

        _building_context = nullptr;
    }
}

const sp<PipelineInput>& PipelineLayout::input() const
{
    return _input;
}

std::map<ShaderStage::Set, String> PipelineLayout::getPreprocessedShaders(const RenderEngineContext& renderEngineContext) const
{
    std::map<ShaderStage::Set, String> shaders;
    const std::map<String, String>& definitions = _definitions;

    for(const auto& [i, j] : _preprocessed_stages)
        shaders[i] = j.toSourceCode(renderEngineContext, definitions);

    return shaders;
}

size_t PipelineLayout::colorAttachmentCount() const
{
    return _color_attachment_count;
}

void PipelineLayout::initialize()
{
    _building_context->initialize();

    if(const ShaderPreprocessor* fragment = _building_context->tryGetStage(ShaderStage::SHADER_STAGE_FRAGMENT))
        _color_attachment_count = fragment->_main_block->outArgumentCount() + (fragment->_main_block->hasReturnValue() ? 1 : 0);

    _input->initialize(_building_context);
}

Table<String, sp<Texture>> PipelineLayout::makeBindingSamplers() const
{
    DASSERT(_building_context);
    const PipelineInput& pipelineInput = _input;
    const Table<String, sp<Texture>>& predefinedSamplers = _building_context->_samplers;
    CHECK_WARN(pipelineInput.samplerCount() >= predefinedSamplers.size(), "Predefined samplers(%d) is more than samplers(%d) in PipelineLayout", predefinedSamplers.size(), _input->samplerCount());

    Table<String, sp<Texture>> samplers;
    for(size_t i = 0; i < pipelineInput.samplerCount(); ++i)
    {
        const String& name = pipelineInput.samplerNames().at(i);
        const auto iter = predefinedSamplers.find(name);
        samplers.push_back(name, iter != predefinedSamplers.end() ? iter->second : (i < predefinedSamplers.size() ? predefinedSamplers.values().at(i) : nullptr));
    }

    return samplers;
}

std::vector<sp<Texture>> PipelineLayout::makeBindingImages() const
{
    DASSERT(_building_context);
    return _building_context->_images.values();
}

const sp<Snippet>& PipelineLayout::snippet() const
{
    return _snippet;
}

}
