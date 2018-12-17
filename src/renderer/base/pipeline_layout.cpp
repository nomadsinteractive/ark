#include "renderer/base/pipeline_layout.h"

#include <regex>

#include "core/base/bean_factory.h"
#include "core/dom/document.h"
#include "core/inf/array.h"
#include "core/inf/flatable.h"
#include "core/util/documents.h"

#include "renderer/base/gl_context.h"
#include "renderer/base/resource_manager.h"
#include "renderer/base/shader_preprocessor.h"
#include "renderer/base/graphics_context.h"
#include "renderer/impl/snippet/snippet_linked_chain.h"
#include "renderer/inf/pipeline_factory.h"
#include "renderer/inf/renderer_factory.h"

#include "renderer/opengl/base/gl_pipeline.h"

namespace ark {

PipelineLayout::PipelineLayout(const sp<RenderController>& renderController, const String& vertex, const String& fragment)
    : _render_controller(renderController), _input(sp<PipelineInput>::make()),
      _preprocessor_context(new PipelineBuildingContext(*this, vertex, fragment))
{
}

void PipelineLayout::loadPredefinedParam(BeanFactory& factory, const sp<Scope>& args, const document& manifest)
{
    loadPredefinedUniform(factory, args, manifest);
    loadPredefinedAttribute(manifest);
}

Attribute& PipelineLayout::addAttribute(const String& name, const String& type, uint32_t scopes)
{
    return _preprocessor_context->addPredefinedAttribute(name, type, scopes);
}

void PipelineLayout::addSnippet(const sp<Snippet>& snippet)
{
    DASSERT(snippet);
    _snippet = _snippet ? sp<Snippet>::adopt(new SnippetLinkedChain(_snippet, snippet)) : snippet;
}

void PipelineLayout::preCompile(GraphicsContext& graphicsContext)
{
    if(_preprocessor_context)
    {
        if(_snippet)
            _snippet->preCompile(graphicsContext, _preprocessor_context);

        _preprocessor_context->preCompile();

        _preprocessor_context->_vertex.insertPredefinedUniforms(_input->uniforms());
        _preprocessor_context->_fragment.insertPredefinedUniforms(_input->uniforms());

        if(graphicsContext.glContext()->version() >= Ark::OPENGL_30)
            _preprocessor_context->_fragment._out_declarations.declare("vec4", "v_", "FragColor");

        _vertex = _preprocessor_context->_vertex.preprocess();
        _fragment = _preprocessor_context->_fragment.preprocess();

        _preprocessor_context.reset();
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

void PipelineLayout::initialize()
{
    DCHECK(_preprocessor_context, "GLShaderSource should not be initialized more than once");

    if(_snippet)
        _snippet->preInitialize(*this);

    _preprocessor_context->initialize();
}

void PipelineLayout::addUniform(const String& name, Uniform::Type type, const sp<Flatable>& flatable, const sp<Changed>& changed)
{
    _input->uniforms().emplace_back(name, type, flatable, changed, _render_controller);
}

const sp<RenderController>& PipelineLayout::renderController() const
{
    return _render_controller;
}

const sp<Snippet>& PipelineLayout::snippet() const
{
    return _snippet;
}

void PipelineLayout::loadPredefinedAttribute(const document& manifest)
{
    for(const document& i : manifest->children("attribute"))
    {
        const String& name = Documents::ensureAttribute(i, Constants::Attributes::NAME);
        DCHECK(!name.empty(), "Empty name");
        DWARN(isupper(name[0]) || name.startsWith("a_"), "GLAttribute name \"%s\" should be capital first or started with a_", name.c_str());
        const String attrName = name.startsWith("a_") ? name.substr(2) : name;
        const String& type = Documents::ensureAttribute(i, Constants::Attributes::TYPE);
        uint32_t divisor = Documents::getAttribute<uint32_t>(i, "divisor", 0);
        addAttribute(attrName, type).setDivisor(divisor);
    }
}

void PipelineLayout::loadPredefinedUniform(BeanFactory& factory, const sp<Scope>& args, const document& manifest)
{
    for(const document& i : manifest->children("uniform"))
    {
        const String& name = Documents::ensureAttribute(i, Constants::Attributes::NAME);
        const String& type = Documents::ensureAttribute(i, Constants::Attributes::TYPE);
        const String& value = Documents::ensureAttribute(i, Constants::Attributes::VALUE);
        const sp<Flatable> flatable = factory.ensure<Flatable>(type, value, args);
        const uint32_t size = flatable->size();
        const uint32_t length = flatable->length();
        Uniform::Type glType = Uniform::UNIFORM_F1;
        switch (size / length) {
        case 4:
            if(type[0] == 'f')
                glType = length > 1 ? Uniform::UNIFORM_F1V : Uniform::UNIFORM_F1;
            else if(type[0] == 'i')
                glType = length > 1 ? Uniform::UNIFORM_I1V : Uniform::UNIFORM_I1;
            else
                FATAL("Unknow type \"%s\"", type.c_str());
            break;
        case 8:
            glType = length > 1 ? Uniform::UNIFORM_F2V : Uniform::UNIFORM_F2;
            break;
        case 12:
            glType = length > 1 ? Uniform::UNIFORM_F3V : Uniform::UNIFORM_F3;
            break;
        case 16:
            glType = length > 1 ? Uniform::UNIFORM_F4V : Uniform::UNIFORM_F4;
            break;
        case 64:
            glType = length > 1 ? Uniform::UNIFORM_MAT4V : Uniform::UNIFORM_MAT4;
            break;
        default:
            FATAL("Unknow type \"%s\"", type.c_str());
            break;
        }
        addUniform(name, glType, flatable, flatable.as<Changed>());
    }
}

}
