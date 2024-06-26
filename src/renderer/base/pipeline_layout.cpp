#include "renderer/base/pipeline_layout.h"

#include "core/inf/uploader.h"

#include "graphics/base/camera.h"

#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/shader_preprocessor.h"
#include "renderer/inf/snippet.h"

namespace ark {

PipelineLayout::PipelineLayout(sp<PipelineBuildingContext> buildingContext)
    : _building_context(std::move(buildingContext)), _input(_building_context->_input), _snippet(_building_context->makePipelineSnippet()),
      _color_attachment_count(0), _definitions(_building_context->toDefinitions())
{
    initialize();
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

const Table<String, sp<Texture>>& PipelineLayout::samplers() const
{
    return _samplers;
}

const std::vector<sp<Texture>>& PipelineLayout::images() const
{
    return _images;
}

void PipelineLayout::initialize()
{
    DCHECK(_building_context, "PipelineLayout should not be initialized more than once");

    _snippet->preInitialize(_building_context);
    _building_context->initialize();

    ShaderPreprocessor* vertex = _building_context->tryGetStage(PipelineInput::SHADER_STAGE_VERTEX);
    ShaderPreprocessor* compute = _building_context->tryGetStage(PipelineInput::SHADER_STAGE_COMPUTE);
    if(vertex)
        tryBindCamera(*vertex, _input->_camera);
    if(compute)
        tryBindCamera(*compute, _input->_camera);

    ShaderPreprocessor* fragment = _building_context->tryGetStage(PipelineInput::SHADER_STAGE_FRAGMENT);
    if(fragment)
        _color_attachment_count = fragment->_main_block->outArgumentCount() + (fragment->_main_block->hasReturnValue() ? 1 : 0);

    _building_context->setupUniforms();

    _input->initialize(_building_context);

    _samplers = makeBindingSamplers();
    _images = makeBindingImages();
}

void PipelineLayout::tryBindUniformMatrix(const ShaderPreprocessor& shaderPreprocessor, String name, sp<Mat4> matrix)
{
    if(sp<Uniform> uniform = shaderPreprocessor.makeUniformInput(std::move(name), Uniform::TYPE_MAT4))
    {
        uniform->setUploader(sp<UploaderOfVariable<M4>>::make(std::move(matrix)));
        _building_context->addUniform(std::move(uniform));
    }
}

void PipelineLayout::tryBindCamera(const ShaderPreprocessor& shaderPreprocessor, const Camera& camera)
{
    tryBindUniformMatrix(shaderPreprocessor, "u_VP", camera.vp());
    tryBindUniformMatrix(shaderPreprocessor, "u_View", camera.view());
    tryBindUniformMatrix(shaderPreprocessor, "u_Projection", camera.projection());
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
