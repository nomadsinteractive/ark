#include "renderer/base/pipeline_layout.h"

#include "core/base/bean_factory.h"
#include "core/base/observer.h"
#include "core/dom/dom_document.h"
#include "core/epi/notifier.h"
#include "core/inf/array.h"
#include "core/inf/flatable.h"
#include "core/util/documents.h"

#include "graphics/base/camera.h"

#include "renderer/base/render_context.h"
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

        _vertex = _building_context->_vertex.preprocess();
        _fragment = _building_context->_fragment.preprocess();

        _building_context = nullptr;
    }
}

const sp<PipelineInput>& PipelineLayout::input() const
{
    return _input;
}

std::map<Shader::Stage, String> PipelineLayout::getPreprocessedShaders(const RenderContext& glContext) const
{
    std::map<Shader::Stage, String> shaders;
    shaders[Shader::STAGE_VERTEX] = _vertex.process(glContext);
    shaders[Shader::STAGE_FRAGMENT] = _fragment.process(glContext);
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
    DCHECK(_building_context, "ShaderSource should not be initialized more than once");

    if(_snippet)
        _snippet->preInitialize(_building_context);

    _building_context->initialize();

    addUniform("u_MVP", camera.vp(), camera.notifier());
    addUniform("u_VP", camera.vp(), camera.notifier());
    addUniform("u_View", camera.view(), camera.notifier());
    addUniform("u_Projection", camera.projection(), camera.notifier());

    _building_context->setupBindings();

    _input->initialize(_building_context);
}

void PipelineLayout::addUniform(const String& name, const sp<Flatable>& flatable, const sp<Notifier>& notifier)
{
    sp<Uniform> uniform = _building_context->_vertex.getUniformInput(name, Uniform::TYPE_MAT4);
    if(uniform)
    {
        uniform->setFlatable(flatable);
        uniform->setNotifier(notifier);
        _building_context->addUniform(uniform);
    }
}

const sp<Snippet>& PipelineLayout::snippet() const
{
    return _snippet;
}

}
