#include "renderer/base/pipeline_layout.h"

#include "core/base/bean_factory.h"
#include "core/base/observer.h"
#include "core/dom/dom_document.h"
#include "core/base/notifier.h"
#include "core/inf/array.h"
#include "core/inf/input.h"
#include "core/util/documents.h"

#include "graphics/base/camera.h"

#include "renderer/base/render_engine_context.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/shader_preprocessor.h"
#include "renderer/inf/pipeline_factory.h"
#include "renderer/inf/renderer_factory.h"
#include "renderer/inf/snippet.h"

namespace ark {

PipelineLayout::PipelineLayout(const sp<PipelineBuildingContext>& buildingContext)
    : _building_context(buildingContext), _input(_building_context->_input), _snippet(_building_context->makePipelineSnippet()), _samplers(_building_context->_samplers), _color_attachment_count(0)
{
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

    for(const auto& i : _preprocessed_stages)
        shaders[i.first] = i.second.toSourceCode(renderEngineContext);

    return shaders;
}

const Table<String, sp<Texture>>& PipelineLayout::samplers() const
{
    return _samplers;
}

uint32_t PipelineLayout::colorAttachmentCount() const
{
    return _color_attachment_count;
}

void PipelineLayout::initialize(const Camera& camera)
{
    DCHECK(_building_context, "PipelineLayout should not be initialized more than once");

    _snippet->preInitialize(_building_context);
    _building_context->initialize();

    ShaderPreprocessor* vertex = _building_context->tryGetStage(PipelineInput::SHADER_STAGE_VERTEX);
    if(vertex)
    {
        tryBindUniform(*vertex, "u_MVP", camera.vp());
        tryBindUniform(*vertex, "u_VP", camera.vp());
        tryBindUniform(*vertex, "u_View", camera.view());
        tryBindUniform(*vertex, "u_Projection", camera.projection());
    }
    ShaderPreprocessor* fragment = _building_context->tryGetStage(PipelineInput::SHADER_STAGE_FRAGMENT);
    if(fragment)
        _color_attachment_count = fragment->_main_block->_outs.size() + (fragment->_main_block->hasReturnValue() ? 1 : 0);


    _building_context->setupUniforms();

    _input->initialize(_building_context);
}

void PipelineLayout::tryBindUniform(const ShaderPreprocessor& shaderPreprocessor, const String& name, const sp<Input>& input)
{
    sp<Uniform> uniform = shaderPreprocessor.getUniformInput(name, Uniform::TYPE_MAT4);
    if(uniform)
    {
        uniform->setInput(input);
        _building_context->addUniform(uniform);
    }
}

const sp<Snippet>& PipelineLayout::snippet() const
{
    return _snippet;
}

}
