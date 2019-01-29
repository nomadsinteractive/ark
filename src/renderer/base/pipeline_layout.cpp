#include "renderer/base/pipeline_layout.h"

#include "core/base/bean_factory.h"
#include "core/epi/notifier.h"
#include "core/base/observer.h"
#include "core/dom/dom_document.h"
#include "core/inf/array.h"
#include "core/inf/flatable.h"
#include "core/util/documents.h"

#include "renderer/base/render_context.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/shader_preprocessor.h"
#include "renderer/base/snippet_delegate.h"
#include "renderer/inf/pipeline_factory.h"
#include "renderer/inf/renderer_factory.h"
#include "renderer/inf/snippet.h"

#include "renderer/opengl/base/gl_pipeline.h"

namespace ark {

PipelineLayout::PipelineLayout(const sp<RenderController>& renderController, const sp<PipelineBuildingContext>& buildingContext)
    : _render_controller(renderController), _building_context(buildingContext), _input(_building_context->_input), _snippet(sp<SnippetDelegate>::make(buildingContext->_snippet))
{
}

void PipelineLayout::preCompile(GraphicsContext& graphicsContext, const sp<ShaderBindings>& bindings)
{
    if(_building_context)
    {
        _snippet->preCompile(graphicsContext, _building_context, bindings);

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

void PipelineLayout::initialize(const Camera& camera)
{
    DCHECK(_building_context, "ShaderSource should not be initialized more than once");

    if(_snippet)
        _snippet->preInitialize(_building_context);

    _building_context->initialize();

    sp<Uniform> mvp = _building_context->_vertex.getUniformInput("u_MVP", Uniform::TYPE_MAT4);
    if(mvp)
    {
        mvp->setFlatable(camera.vp());
        mvp->setNotifier(camera.notifier());
        _building_context->addUniform(mvp);
    }

    sp<Uniform> vp = _building_context->_vertex.getUniformInput("u_VP", Uniform::TYPE_MAT4);
    if(vp)
    {
        vp->setFlatable(camera.vp());
        vp->setNotifier(camera.notifier());
        _building_context->addUniform(vp);
    }

    _building_context->setupBindings();

    _input->initialize(_building_context);
}

const sp<RenderController>& PipelineLayout::renderController() const
{
    return _render_controller;
}

const sp<Snippet>& PipelineLayout::snippet() const
{
    return _snippet;
}

}
