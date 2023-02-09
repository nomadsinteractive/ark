#include "renderer/base/pipeline_layout.h"

#include "core/inf/input.h"

#include "graphics/base/camera.h"

#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/shader_preprocessor.h"
#include "renderer/inf/snippet.h"

namespace ark {

PipelineLayout::PipelineLayout(const sp<PipelineBuildingContext>& buildingContext, const Camera& camera)
    : _building_context(buildingContext), _input(_building_context->_input), _snippet(_building_context->makePipelineSnippet()), _color_attachment_count(0), _definitions(_building_context->toDefinitions())
{
    initialize(camera);
}

void PipelineLayout::preCompile(GraphicsContext& graphicsContext)
{
    if(_building_context)
    {
        _snippet->preCompile(graphicsContext, _building_context, *this);

        for(const auto& i : _building_context->stages())
            _preprocessed_stages[i.first] = i.second->preprocess();

        _building_context = nullptr;
    }
}

const sp<PipelineInput>& PipelineLayout::input() const
{
    return _input;
}

std::map<PipelineInput::ShaderStage, String> PipelineLayout::getPreprocessedShaders(const RenderEngineContext& renderEngineContext) const
{
    std::map<PipelineInput::ShaderStage, String> shaders;
    const std::map<String, String>& definitions = _definitions;

    for(const auto& [i, j] : _preprocessed_stages)
        shaders[i] = j.toSourceCode(renderEngineContext, definitions);

    return shaders;
}

size_t PipelineLayout::colorAttachmentCount() const
{
    return _color_attachment_count;
}

const std::vector<sp<Texture>>& PipelineLayout::samplers() const
{
    return _samplers;
}

const std::vector<sp<Texture>>& PipelineLayout::images() const
{
    return _images;
}

void PipelineLayout::initialize(const Camera& camera)
{
    DCHECK(_building_context, "PipelineLayout should not be initialized more than once");

    _snippet->preInitialize(_building_context);
    _building_context->initialize();

    ShaderPreprocessor* vertex = _building_context->tryGetStage(PipelineInput::SHADER_STAGE_VERTEX);
    ShaderPreprocessor* compute = _building_context->tryGetStage(PipelineInput::SHADER_STAGE_COMPUTE);
    if(vertex)
        tryBindCamera(*vertex, camera);
    if(compute)
        tryBindCamera(*compute, camera);

    ShaderPreprocessor* fragment = _building_context->tryGetStage(PipelineInput::SHADER_STAGE_FRAGMENT);
    if(fragment)
        _color_attachment_count = fragment->_main_block->outArgumentCount() + (fragment->_main_block->hasReturnValue() ? 1 : 0);

    _building_context->setupUniforms();

    _input->initialize(_building_context);

    _samplers = makeBindingSamplers();
    _images = makeBindingImages();
}

void PipelineLayout::tryBindUniform(const ShaderPreprocessor& shaderPreprocessor, const String& name, const sp<Input>& input)
{
    sp<Uniform> uniform = shaderPreprocessor.getUniformInput(name, Uniform::TYPE_MAT4);
    if(uniform)
    {
        uniform->setInput(input);
        _building_context->addUniform(std::move(uniform));
    }
}

void PipelineLayout::tryBindCamera(const ShaderPreprocessor& shaderPreprocessor, const Camera& camera)
{
    tryBindUniform(shaderPreprocessor, "u_VP", camera.vp());
    tryBindUniform(shaderPreprocessor, "u_View", camera.view());
    tryBindUniform(shaderPreprocessor, "u_Projection", camera.projection());
}

std::vector<sp<Texture>> PipelineLayout::makeBindingSamplers() const
{
    DASSERT(_building_context);
    std::vector<sp<Texture>> bs(_input->samplerCount());
    const Table<String, sp<Texture>>& samplers = _building_context->_samplers;
    CHECK_WARN(bs.size() >= samplers.size(), "Predefined samplers(%d) is more than samplers(%d) in PipelineLayout", samplers.size(), bs.size());

    for(size_t i = 0; i < samplers.values().size(); ++i)
        if(i < bs.size())
            bs[i] = samplers.values().at(i);

    return bs;
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
