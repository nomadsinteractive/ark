#include "renderer/base/pipeline_layout.h"

#include <regex>

#include "core/base/bean_factory.h"
#include "core/dom/document.h"
#include "core/inf/array.h"
#include "core/inf/flatable.h"
#include "core/util/documents.h"

#include "renderer/base/gl_context.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/resource_manager.h"
#include "renderer/base/shader_preprocessor.h"
#include "renderer/inf/pipeline_factory.h"
#include "renderer/inf/renderer_factory.h"
#include "renderer/inf/snippet.h"

#include "renderer/opengl/base/gl_pipeline.h"

namespace ark {

PipelineLayout::PipelineLayout(const sp<RenderController>& renderController, const sp<PipelineBuildingContext>& buildingContext)
    : _render_controller(renderController), _building_context(buildingContext), _input(_building_context->_input), _snippet(buildingContext->_snippet)
{
}

void PipelineLayout::preCompile(GraphicsContext& graphicsContext)
{
    if(_building_context)
    {
        if(_snippet)
            _snippet->preCompile(graphicsContext, _building_context);

        _building_context->preCompile();

        _building_context->_vertex.insertPredefinedUniforms(_input->uniforms());
        _building_context->_fragment.insertPredefinedUniforms(_input->uniforms());

        if(graphicsContext.glContext()->version() >= Ark::OPENGL_30)
            _building_context->_fragment._out_declarations.declare("vec4", "v_", "FragColor");

        _vertex = _building_context->_vertex.preprocess();
        _fragment = _building_context->_fragment.preprocess();

        _building_context = nullptr;
    }
}

const sp<PipelineInput>& PipelineLayout::input() const
{
    return _input;
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
    DCHECK(_building_context, "GLShaderSource should not be initialized more than once");

    if(_snippet)
        _snippet->preInitialize(_building_context);

    _building_context->initialize();

    if(_building_context->_vertex._uniforms.has("MVP"))
    {

    }
    _input->initialize(_building_context->_uniforms.values());
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
