#include "renderer/base/gl_shader_source.h"

#include <regex>

#include "core/base/bean_factory.h"
#include "core/dom/document.h"
#include "core/inf/array.h"
#include "core/types/global.h"
#include "core/util/documents.h"

#include "renderer/base/gl_context.h"
#include "renderer/base/gl_program.h"
#include "renderer/base/graphics_context.h"
#include "renderer/impl/gl_snippet/gl_snippet_linked_chain.h"
#include "renderer/util/gl_shader_preprocessor.h"


namespace ark {

GLShaderSource::GLShaderSource(const String& vertex, const String& fragment, const sp<ResourceLoaderContext::Synchronizer>& synchronizer)
    : _vertex(GLShaderPreprocessor::SHADER_TYPE_VERTEX, vertex), _fragment(GLShaderPreprocessor::SHADER_TYPE_FRAGMENT, fragment), _stride(0), _synchronizer(synchronizer)
{
}

void GLShaderSource::loadPredefinedParam(BeanFactory& factory, const sp<Scope>& args, const document& manifest)
{
    loadPredefinedUniform(factory, args, manifest);
    loadPredefinedAttribute(manifest);
}

String& GLShaderSource::vertex()
{
    return _vertex._source;
}

const String& GLShaderSource::vertex() const
{
    return _vertex._source;
}

String& GLShaderSource::fragment()
{
    return _fragment._source;
}

const String& GLShaderSource::fragment() const
{
    return _fragment._source;
}

sp<GLProgram> GLShaderSource::makeGLProgram(GraphicsContext& graphicsContext) const
{
    const sp<GLContext>& glContext = graphicsContext.glContext();
    return sp<GLProgram>::make(glContext->getGLSLVersion(), _vertex.process(glContext), _fragment.process(glContext));
}

void GLShaderSource::addPredefinedAttribute(const String& name, const String& type)
{
    if(_attributes.find(name) == _attributes.end())
        _attributes[name] = getPredefinedAttribute(name, type);
}

void GLShaderSource::addSnippet(const sp<GLSnippet>& snippet)
{
    NOT_NULL(snippet);
    _snippet = _snippet ? sp<GLSnippet>::adopt(new GLSnippetLinkedChain(_snippet, snippet)) : snippet;
}

GLShader::Slot GLShaderSource::preprocess(GraphicsContext& graphicsContext)
{
    if(graphicsContext.glContext()->version() >= Ark::OPENGL_30)
        _fragment._out_declarations.declare("vec4", "v_", "FragColor");

    _vertex.preprocess();
    _fragment.preprocess();

    return GLShader::Slot(_vertex._source, _fragment._source);
}

void GLShaderSource::initialize()
{
    GLShaderPreprocessor::Context context;

    if(_snippet)
        _snippet->preCompile(*this, context);

    _vertex.parse1(*this);
    _fragment.parse1(*this);
    _vertex.parse2(context, *this);
    _fragment.parse2(context, *this);

    for(const auto& i : _vertex._in_declarations._declared)
        if(context.vertexInsDeclared.find(i.first) == context.vertexInsDeclared.end())
        {
            context.vertexInsDeclared[i.first] = i.second;
            addAttribute(i.second, i.first);
        }

    std::map<String, String> attributes = _fragment._in_declarations._declared;
    if(_snippet)
        for(const auto& iter : _attributes)
            attributes[iter.first] = iter.second.type();

    StringBuilder vertMainSource;
    StringBuilder fragColorModifier;
    StringBuilder fragProcedures;
    StringBuilder fragProcedureCalls;

    std::set<String> fragmentUsedVars;
    static const std::regex VAR_PATTERN("\\bv_([\\w\\d_]+)\\b");
    _fragment._source.search(VAR_PATTERN, [&fragmentUsedVars](const std::smatch& m)->bool {
        fragmentUsedVars.insert(m[1].str());
        return true;
    });

    List<String> generated;

    for(const auto& i : context.vertexIns)
        _vertex._in_declarations.declare(i.first, "a_", Strings::capitalFirst(i.second));
    for(const auto& i : context.fragmentIns)
    {
        const String n = Strings::capitalFirst(i.second);
        fragmentUsedVars.insert(n);
        _fragment._in_declarations.declare(i.first, "v_", n);
    }

    for(const auto& iter : attributes)
    {
        if(!_vertex._in_declarations.has(iter.first) && !_vertex._out_declarations.has(iter.first))
        {
            generated.push_back(iter.first);
            addAttribute(iter.first, iter.second);
        }
    }
    uint32_t mod = _stride % sizeof(GLfloat);
    if(mod != 0)
        _stride += (sizeof(GLfloat) - mod);

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
                vertMainSource << "v_" << i << " = " << "a_" << i << ";\n";
            }
        }
    }

    for(const auto& i : context.vertexOuts)
        _vertex._out_declarations.declare(i.first, "", i.second);

    for(const GLShaderPreprocessor::Snippet& i : context._vertex_snippets)
    {
        switch(i._type)
        {
            case GLShaderPreprocessor::SNIPPET_TYPE_SOURCE:
                vertMainSource << i._src << '\n';
            break;
            default:
            break;
        }
    }

    for(const GLShaderPreprocessor::Snippet& i : context._fragment_snippets)
    {
        switch(i._type)
        {
            case GLShaderPreprocessor::SNIPPET_TYPE_MULTIPLY:
                fragColorModifier << " * " << i._src;
            break;
            case GLShaderPreprocessor::SNIPPET_TYPE_PROCEDURE:
                fragProcedures << i._src;
            break;
            case GLShaderPreprocessor::SNIPPET_TYPE_PROCEDURE_CALL:
                fragProcedureCalls << i._src;
            break;
            default:
                break;
        }
    }

    if(fragColorModifier.dirty())
    {
        String::size_type pos = _fragment._source.rfind(';');
        DCHECK(pos != String::npos, "Cannot find fragment color modifier point, empty fragment shader?");
        _fragment._source.insert(pos, fragColorModifier.str());
    }
    if(fragProcedures.dirty())
        insertBefore(_fragment._source, "void main()", fragProcedures.str());
    if(fragProcedureCalls.dirty())
    {
        String::size_type pos = _fragment._source.rfind(';');
        DCHECK(pos != String::npos, "Cannot find fragment color modifier point, empty fragment shader?");
        _fragment._source.insert(pos + 1, fragProcedureCalls.str());
    }

    if(vertMainSource.dirty())
    {
        vertMainSource << "    ";
        insertBefore(_vertex._source, "gl_Position =", vertMainSource.str());
    }

    _vertex.insertPredefinedUniforms(_uniforms);
    _fragment.insertPredefinedUniforms(_uniforms);

}

void GLShaderSource::addAttribute(const String& name, const String& type)
{
    if(_attributes.find(name) == _attributes.end())
        addPredefinedAttribute(name, type);

    GLAttribute& attr = _attributes[name];
    attr.setOffset(_stride);
    _stride += attr.size();
}

void GLShaderSource::addUniform(const String& name, GLUniform::Type type, const sp<Flatable>& flatable, const sp<Changed>& changed)
{
    _uniforms.push_back(GLUniform(name, type, flatable, changed, _synchronizer));
}

const sp<GLSnippet>& GLShaderSource::snippet() const
{
    return _snippet;
}

void GLShaderSource::loadPredefinedAttribute(const document& manifest)
{
    for(const document& i : manifest->children("attribute"))
    {
        const String& name = Documents::ensureAttribute(i, Constants::Attributes::NAME);
        const String attrName = name.startsWith("a_") ? name.substr(2) : name;
        const String& type = Documents::ensureAttribute(i, Constants::Attributes::TYPE);
        addPredefinedAttribute(attrName, type);
    }
}

void GLShaderSource::loadPredefinedUniform(BeanFactory& factory, const sp<Scope>& args, const document& manifest)
{
    for(const document& i : manifest->children("uniform"))
    {
        const String& name = Documents::ensureAttribute(i, Constants::Attributes::NAME);
        const String& type = Documents::ensureAttribute(i, Constants::Attributes::TYPE);
        const String& value = Documents::ensureAttribute(i, Constants::Attributes::VALUE);
        String::size_type pos = type.find('[');
        bool isArray = pos != String::npos;
        const String stype = isArray ? type.substr(0, pos) : type;
        const sp<Flatable> flatable = factory.ensure<Flatable>(stype, value, args);
        GLUniform::Type glType = GLUniform::UNIFORM_F1;
        if(stype == "float")
            glType = isArray ? GLUniform::UNIFORM_F1V : GLUniform::UNIFORM_F1;
        else if(stype == "vec2")
            glType = isArray ? GLUniform::UNIFORM_F2V : GLUniform::UNIFORM_F2;
        else if(stype == "vec3")
            glType = isArray ? GLUniform::UNIFORM_F3V : GLUniform::UNIFORM_F3;
        else if(stype == "v4f")
            glType = isArray ? GLUniform::UNIFORM_F4V : GLUniform::UNIFORM_F4;
        else if(stype == "mat")
            glType = isArray ? GLUniform::UNIFORM_MAT4V : GLUniform::UNIFORM_MAT4;
        else
            FATAL("Unknow type \"%s\"", type.c_str());
        addUniform(name, glType, flatable, flatable.as<Changed>());
    }
}

GLAttribute GLShaderSource::getPredefinedAttribute(const String& name, const String& type)
{
    if(type == "vec3")
        return GLAttribute("a_" + name, type, GL_FLOAT, 3, GL_FALSE);
    if(name == "TexCoordinate")
        return GLAttribute("a_TexCoordinate", type, GL_UNSIGNED_SHORT, 2, GL_TRUE);
    if(type == "vec2")
        return GLAttribute("a_" + name, type, GL_FLOAT, 2, GL_FALSE);
    if(name == "Position")
        return GLAttribute("a_Position", type, GL_FLOAT, 3, GL_FALSE);
    if(type == "float")
        return GLAttribute("a_" + name, type, GL_FLOAT, 1, GL_FALSE);
    if(type == "vec4")
        return GLAttribute("a_" + name, type, GL_FLOAT, 4, GL_FALSE);
    if(type == "color3b")
        return GLAttribute("a_" + name, type, GL_UNSIGNED_BYTE, 3, GL_TRUE);
    if(type == "uint8")
        return GLAttribute("a_" + name, type, GL_UNSIGNED_BYTE, 1, GL_FALSE);
    DFATAL("Unknown attribute type \"%s\"", type.c_str());
    return GLAttribute();
}

void GLShaderSource::insertBefore(String& src, const String& statement, const String& str)
{
    String::size_type pos = src.find(statement);
    if(pos != String::npos)
        src.insert(pos, str);
}

}
