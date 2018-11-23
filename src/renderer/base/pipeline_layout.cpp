#include "renderer/base/pipeline_layout.h"

#include <regex>

#include "core/base/bean_factory.h"
#include "core/dom/document.h"
#include "core/inf/array.h"
#include "core/inf/flatable.h"
#include "core/types/global.h"
#include "core/util/documents.h"

#include "renderer/base/gl_context.h"
#include "renderer/base/gl_program.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/gl_shader_preprocessor.h"
#include "renderer/base/graphics_context.h"
#include "renderer/impl/gl_snippet/gl_snippet_linked_chain.h"
#include "renderer/inf/renderer_factory.h"


namespace ark {

PipelineLayout::PipelineLayout(const sp<RenderController>& renderController, const String& vertex, const String& fragment)
    : _pipeline_factory(renderController->renderEngine()->rendererFactory()->createPipelineFactory()), _preprocessor_context(new GLShaderPreprocessorContext()),
      _vertex(GLShaderPreprocessor::SHADER_TYPE_VERTEX, vertex),
      _fragment(GLShaderPreprocessor::SHADER_TYPE_FRAGMENT, fragment),
      _render_controller(renderController), _input(sp<PipelineInput>::make())
{
}

void PipelineLayout::loadPredefinedParam(BeanFactory& factory, const sp<Scope>& args, const document& manifest)
{
    loadPredefinedUniform(factory, args, manifest);
    loadPredefinedAttribute(manifest);
}

sp<GLProgram> PipelineLayout::makeGLProgram(GraphicsContext& graphicsContext) const
{
    const sp<GLContext>& glContext = graphicsContext.glContext();
    return sp<GLProgram>::make(graphicsContext.glResourceManager()->recycler(), glContext->getGLSLVersion(), _vertex.process(glContext), _fragment.process(glContext));
}

GLAttribute& PipelineLayout::addPredefinedAttribute(const String& name, const String& type, uint32_t scopes)
{
    return _preprocessor_context->addPredefinedAttribute(name, type, scopes);
}

void PipelineLayout::addSnippet(const sp<GLSnippet>& snippet)
{
    DASSERT(snippet);
    _snippet = _snippet ? sp<GLSnippet>::adopt(new GLSnippetLinkedChain(_snippet, snippet)) : snippet;
}

void PipelineLayout::preprocess(GraphicsContext& graphicsContext)
{
    if(_preprocessor_context)
    {
        if(_snippet)
            _snippet->preCompile(graphicsContext, _preprocessor_context);

        _preprocessor_context->precompile(_vertex._source, _fragment._source);

        _vertex.insertPredefinedUniforms(_input->uniforms());
        _fragment.insertPredefinedUniforms(_input->uniforms());

        if(graphicsContext.glContext()->version() >= Ark::OPENGL_30)
            _fragment._out_declarations.declare("vec4", "v_", "FragColor");

        _vertex.preprocess();
        _fragment.preprocess();

        _preprocessor_context.reset();
    }
}

GLShaderPreprocessor& PipelineLayout::vertex()
{
    return _vertex;
}

GLShaderPreprocessor& PipelineLayout::fragment()
{
    return _fragment;
}

const sp<PipelineInput>& PipelineLayout::input() const
{
    return _input;
}

void PipelineLayout::initialize()
{
    DCHECK(_preprocessor_context, "GLShaderSource should not be initialized more than once");

    GLShaderPreprocessorContext& context = _preprocessor_context;

    if(_snippet)
        _snippet->preInitialize(*this);

    _vertex.parseMainBlock(*this);
    _vertex.parseDeclarations(context, *this);

    _fragment.parseMainBlock(*this);
    _fragment.parseDeclarations(context, *this);

    for(const auto& i : _vertex._in_declarations._declared)
        if(context._vert_in_declared.find(i.first) == context._vert_in_declared.end())
        {
            context._vert_in_declared[i.first] = i.second;
            addAttribute(i.second, i.first);
        }

    std::set<String> fragmentUsedVars;
    static const std::regex VAR_PATTERN("\\bv_([\\w\\d_]+)\\b");
    _fragment._source.search(VAR_PATTERN, [&fragmentUsedVars](const std::smatch& m)->bool {
        fragmentUsedVars.insert(m[1].str());
        return true;
    });

    for(const auto& i : context._vertex_in)
        _vertex._in_declarations.declare(i.first, "a_", Strings::capitalFirst(i.second));
    for(const auto& i : context._fragment_in)
    {
        const String n = Strings::capitalFirst(i.second);
        fragmentUsedVars.insert(n);
        _fragment._in_declarations.declare(i.first, "v_", n);
    }

    std::map<String, String> attributes = _fragment._in_declarations._declared;
    if(_snippet)
        for(const auto& iter : context._attributes)
            attributes[iter.first] = iter.second.type();

    List<String> generated;
    for(const auto& iter : attributes)
    {
        if(!_vertex._in_declarations.has(iter.first)
                && !_vertex._out_declarations.has(iter.first)
                && !_vertex._main_block->hasOutParam(iter.first))
        {
            generated.push_back(iter.first);
            addAttribute(iter.first, iter.second);
        }
    }

    for(auto iter : _input->_streams)
        iter.second.align();

    for(const auto& i : attributes)
    {
        if(fragmentUsedVars.find(i.first) != fragmentUsedVars.end())
            _fragment._in_declarations.declare(i.second, "v_", i.first);
    }

    if(generated.size())
    {
        for(const String& i : generated)
        {
            _vertex._in_declarations.declare(attributes[i], "a_", i);
            if(fragmentUsedVars.find(i) != fragmentUsedVars.end())
            {
                _vertex._out_declarations.declare(attributes[i], "v_", i);
                context._vert_main_source << "v_" << i << " = " << "a_" << i << ";\n";
            }
        }
    }

    for(const auto& i : context._vertex_out)
        _vertex._out_declarations.declare(i.first, "", i.second);

}

void PipelineLayout::addAttribute(const String& name, const String& type)
{
    GLAttribute& attr = addPredefinedAttribute(name, type);
    _input->addAttribute(name, attr);
}

void PipelineLayout::addUniform(const String& name, GLUniform::Type type, const sp<Flatable>& flatable, const sp<Changed>& changed)
{
    _input->uniforms().emplace_back(name, type, flatable, changed, _render_controller);
}

const sp<GLSnippet>& PipelineLayout::snippet() const
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
        addPredefinedAttribute(attrName, type).setDivisor(divisor);
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
        GLUniform::Type glType = GLUniform::UNIFORM_F1;
        switch (size / length) {
        case 4:
            if(type[0] == 'f')
                glType = length > 1 ? GLUniform::UNIFORM_F1V : GLUniform::UNIFORM_F1;
            else if(type[0] == 'i')
                glType = length > 1 ? GLUniform::UNIFORM_I1V : GLUniform::UNIFORM_I1;
            else
                FATAL("Unknow type \"%s\"", type.c_str());
            break;
        case 8:
            glType = length > 1 ? GLUniform::UNIFORM_F2V : GLUniform::UNIFORM_F2;
            break;
        case 12:
            glType = length > 1 ? GLUniform::UNIFORM_F3V : GLUniform::UNIFORM_F3;
            break;
        case 16:
            glType = length > 1 ? GLUniform::UNIFORM_F4V : GLUniform::UNIFORM_F4;
            break;
        case 64:
            glType = length > 1 ? GLUniform::UNIFORM_MAT4V : GLUniform::UNIFORM_MAT4;
            break;
        default:
            FATAL("Unknow type \"%s\"", type.c_str());
            break;
        }
        addUniform(name, glType, flatable, flatable.as<Changed>());
    }
}

}
