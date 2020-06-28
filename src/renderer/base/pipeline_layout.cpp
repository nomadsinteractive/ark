#include "renderer/base/pipeline_layout.h"

#include "core/base/bean_factory.h"
#include "core/base/observer.h"
#include "core/dom/dom_document.h"
#include "core/epi/notifier.h"
#include "core/inf/array.h"
#include "core/inf/flatable.h"
#include "core/util/documents.h"

#include "graphics/base/camera.h"

#include "renderer/base/render_engine_context.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/shader_preprocessor.h"
#include "renderer/base/snippet_delegate.h"
#include "renderer/inf/pipeline_factory.h"
#include "renderer/inf/renderer_factory.h"
#include "renderer/inf/snippet.h"

namespace ark {

PipelineLayout::PipelineLayout(const sp<PipelineBuildingContext>& buildingContext)
    : _building_context(buildingContext), _input(_building_context->_input), _snippet(sp<SnippetDelegate>::make(buildingContext->_snippet)), _samplers(buildingContext->_samplers)
{
}

void PipelineLayout::preCompile(GraphicsContext& graphicsContext)
{
    if(_building_context)
    {
        _snippet->preCompile(graphicsContext, _building_context, *this);

        _vertex = _building_context->getStage(Shader::SHADER_STAGE_VERTEX)->preprocess();
        _fragment = _building_context->getStage(Shader::SHADER_STAGE_FRAGMENT)->preprocess();

        _building_context = nullptr;
    }
}

const sp<PipelineInput>& PipelineLayout::input() const
{
    return _input;
}

std::map<Shader::Stage, String> PipelineLayout::getPreprocessedShaders(const RenderEngineContext& renderEngineContext) const
{
    std::map<Shader::Stage, String> shaders;
    shaders[Shader::SHADER_STAGE_VERTEX] = _vertex.process(renderEngineContext);
    shaders[Shader::SHADER_STAGE_FRAGMENT] = _fragment.process(renderEngineContext);
    return shaders;
}

const ShaderPreprocessor::Preprocessor& PipelineLayout::vertex() const
{
    return _vertex;
}

const ShaderPreprocessor::Preprocessor& PipelineLayout::fragment() const
{
    return _fragment;
}

const Table<String, sp<Texture>>& PipelineLayout::samplers() const
{
    return _samplers;
}

void PipelineLayout::initialize(const Camera& camera)
{
    DCHECK(_building_context, "PipelineLayout should not be initialized more than once");

    if(_snippet)
        _snippet->preInitialize(_building_context);

    _building_context->initialize();

    if(_building_context->hasStage(Shader::SHADER_STAGE_VERTEX))
    {
        ShaderPreprocessor& vertex = _building_context->getStage(Shader::SHADER_STAGE_VERTEX);
        tryBindUniform(vertex, "u_MVP", camera.vp());
        tryBindUniform(vertex, "u_VP", camera.vp());
        tryBindUniform(vertex, "u_View", camera.view());
        tryBindUniform(vertex, "u_Projection", camera.projection());
    }

    _building_context->setupUniforms();

    _input->initialize(_building_context);
}

void PipelineLayout::tryBindUniform(const ShaderPreprocessor& shaderPreprocessor, const String& name, const sp<Flatable>& flatable)
{
    sp<Uniform> uniform = shaderPreprocessor.getUniformInput(name, Uniform::TYPE_MAT4);
    if(uniform)
    {
        uniform->setFlatable(flatable);
        _building_context->addUniform(uniform);
    }
}

const sp<Snippet>& PipelineLayout::snippet() const
{
    return _snippet;
}

}
