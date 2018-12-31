#include "renderer/base/pipeline_building_context.h"

#include <regex>

#include "core/base/string_buffer.h"
#include "core/epi/changed.h"
#include "core/inf/flatable.h"
#include "core/util/strings.h"

#include "renderer/base/gl_context.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/impl/snippet/snippet_linked_chain.h"

namespace ark {

PipelineBuildingContext::PipelineBuildingContext(const sp<PipelineFactory>& pipelineFactory, const String& vertex, const String& fragment)
    : _shader(sp<Shader::Stub>::make(pipelineFactory)), _input(sp<PipelineInput>::make()), _vertex(ShaderPreprocessor::SHADER_TYPE_VERTEX, vertex), _fragment(ShaderPreprocessor::SHADER_TYPE_FRAGMENT, fragment)
{
}

void PipelineBuildingContext::loadPredefinedParam(BeanFactory& factory, const sp<Scope>& args, const document& manifest)
{
    loadPredefinedUniform(factory, args, manifest);
    loadPredefinedAttribute(manifest);
}

void PipelineBuildingContext::initialize()
{
    _vertex.parse(*this);
    _fragment.parse(*this);

    for(const auto& i : _vertex._in_declarations._declared)
        if(_vert_in_declared.find(i.first) == _vert_in_declared.end())
        {
            _vert_in_declared[i.first] = i.second;
            addAttribute(i.second, i.first);
        }

    std::set<String> fragmentUsedVars;
    static const std::regex varPattern("\\bv_([\\w\\d_]+)\\b");
    _fragment._source.search(varPattern, [&fragmentUsedVars](const std::smatch& m)->bool {
        fragmentUsedVars.insert(m[1].str());
        return true;
    });

    for(const auto& i : _vertex_in)
        _vertex._in_declarations.declare(i.first, "a_", Strings::capitalFirst(i.second));
    for(const auto& i : _fragment_in)
    {
        const String n = Strings::capitalFirst(i.second);
        fragmentUsedVars.insert(n);
        _fragment._in_declarations.declare(i.first, "v_", n);
    }

    std::map<String, String> attributes = _fragment._in_declarations._declared;
    for(const auto& i : _attributes)
        attributes[i.first] = i.second.declareType();

    std::vector<String> generated;
    for(const auto& i : attributes)
    {
        if(!_vertex._in_declarations.has(i.first)
                && !_vertex._out_declarations.has(i.first)
                && !_vertex._main_block->hasOutParam(i.first))
        {
            generated.push_back(i.first);
            addAttribute(i.first, i.second);
        }
    }

    for(auto iter : _input->streams())
        iter.second.align();

    for(const auto& i : attributes)
    {
        if(fragmentUsedVars.find(i.first) != fragmentUsedVars.end())
            _fragment._in_declarations.declare(i.second, "v_", i.first);
    }

    for(const String& i : generated)
    {
        _vertex._in_declarations.declare(attributes[i], "a_", i);
        if(fragmentUsedVars.find(i) != fragmentUsedVars.end())
        {
            _vertex._out_declarations.declare(attributes[i], "v_", i);
            _vert_main_source << "v_" << i << " = " << "a_" << i << ";\n";
        }
    }

    for(const auto& i : _vertex_out)
        _vertex._out_declarations.declare(i.first, "", i.second);
}

void PipelineBuildingContext::preCompile()
{
    doSnippetPrecompile();
    doPrecompile(_vertex, _fragment);
}

void PipelineBuildingContext::addAttribute(const String& name, const String& type)
{
    Attribute& attr = addPredefinedAttribute(name, type, 0);
    _input->addAttribute(name, attr);
}

void PipelineBuildingContext::addSnippet(const sp<Snippet>& snippet)
{
    DASSERT(snippet);
    _snippet = _snippet ? sp<Snippet>::adopt(new SnippetLinkedChain(_snippet, snippet)) : snippet;
}

void PipelineBuildingContext::addUniform(const String& name, Uniform::Type type, const sp<Flatable>& flatable, const sp<Changed>& changed)
{
    _uniforms.push_back(name, Uniform(name, type, flatable, changed));
}

void PipelineBuildingContext::addUniform(Uniform uniform)
{
    String name = uniform.name();
    _uniforms.push_back(std::move(name), std::move(uniform));
}

Attribute& PipelineBuildingContext::addPredefinedAttribute(const String& name, const String& type, uint32_t scopes)
{
    if(_attributes.find(name) == _attributes.end())
        _attributes[name] = makePredefinedAttribute(name, type);

    if(scopes == ShaderPreprocessor::SHADER_TYPE_FRAGMENT)
        _fragment_in.push_back(std::pair<String, String>(type, name));

    return _attributes[name];
}

void PipelineBuildingContext::loadPredefinedAttribute(const document& manifest)
{
    for(const document& i : manifest->children("attribute"))
    {
        const String& name = Documents::ensureAttribute(i, Constants::Attributes::NAME);
        DCHECK(!name.empty(), "Empty name");
        DWARN(isupper(name[0]) || name.startsWith("a_"), "GLAttribute name \"%s\" should be capital first or started with a_", name.c_str());
        const String attrName = name.startsWith("a_") ? name.substr(2) : name;
        const String& type = Documents::ensureAttribute(i, Constants::Attributes::TYPE);
        uint32_t divisor = Documents::getAttribute<uint32_t>(i, "divisor", 0);
        addPredefinedAttribute(attrName, type, 0).setDivisor(divisor);
    }
}

void PipelineBuildingContext::loadPredefinedUniform(BeanFactory& factory, const sp<Scope>& args, const document& manifest)
{
    for(const document& i : manifest->children("uniform"))
    {
        const String& name = Documents::ensureAttribute(i, Constants::Attributes::NAME);
        const String& type = Documents::ensureAttribute(i, Constants::Attributes::TYPE);
        const String& value = Documents::ensureAttribute(i, Constants::Attributes::VALUE);
        const sp<Flatable> flatable = factory.ensure<Flatable>(type, value, args);
        const uint32_t size = flatable->size();
        const uint32_t length = flatable->length();
        Uniform::Type glType = Uniform::TYPE_F1;
        switch (size / length) {
        case 4:
            if(type[0] == 'f')
                glType = length > 1 ? Uniform::TYPE_F1V : Uniform::TYPE_F1;
            else if(type[0] == 'i')
                glType = length > 1 ? Uniform::TYPE_I1V : Uniform::TYPE_I1;
            else
                FATAL("Unknow type \"%s\"", type.c_str());
            break;
        case 8:
            glType = length > 1 ? Uniform::TYPE_F2V : Uniform::TYPE_F2;
            break;
        case 12:
            glType = length > 1 ? Uniform::TYPE_F3V : Uniform::TYPE_F3;
            break;
        case 16:
            glType = length > 1 ? Uniform::TYPE_F4V : Uniform::TYPE_F4;
            break;
        case 64:
            glType = length > 1 ? Uniform::TYPE_MAT4V : Uniform::TYPE_MAT4;
            break;
        default:
            FATAL("Unknow type \"%s\"", type.c_str());
        }
        addUniform(name, glType, flatable, flatable.as<Changed>());
    }
}

Attribute PipelineBuildingContext::makePredefinedAttribute(const String& name, const String& type)
{
    if(type == "vec3")
        return Attribute("a_" + name, Attribute::TYPE_FLOAT, type, 3, false);
    if(name == "TexCoordinate")
        return Attribute("a_TexCoordinate", Attribute::TYPE_USHORT, type, 2, true);
    if(type == "vec2")
        return Attribute("a_" + name, Attribute::TYPE_FLOAT, type, 2, false);
    if(name == "Position")
        return Attribute("a_Position", Attribute::TYPE_FLOAT, type, 3, false);
    if(type == "float")
        return Attribute("a_" + name, Attribute::TYPE_FLOAT, type, 1, false);
    if(type == "vec4")
        return Attribute("a_" + name, Attribute::TYPE_FLOAT, type, 4, false);
    if(type == "color3b")
        return Attribute("a_" + name, Attribute::TYPE_UBYTE, type, 3, false);
    if(type == "uint8")
        return Attribute("a_" + name, Attribute::TYPE_UBYTE, type, 1, false);
    if(type == "mat4")
        return Attribute("a_" + name, Attribute::TYPE_FLOAT, type, 16, false);
    DFATAL("Unknown attribute type \"%s\"", type.c_str());
    return Attribute();
}

void PipelineBuildingContext::doSnippetPrecompile()
{
    for(const ShaderPreprocessor::Snippet& i : _vertex._snippets)
    {
        switch(i._type)
        {
        case ShaderPreprocessor::SNIPPET_TYPE_SOURCE:
            _vert_main_source << i._src << '\n';
            break;
        default:
            break;
        }
    }

    for(const ShaderPreprocessor::Snippet& i : _fragment._snippets)
    {
        switch(i._type)
        {
        case ShaderPreprocessor::SNIPPET_TYPE_MULTIPLY:
            _frag_color_modifier << " * " << i._src;
            break;
        default:
            break;
        }
    }
}

void PipelineBuildingContext::doPrecompile(ShaderPreprocessor& vertex, ShaderPreprocessor& fragment)
{
    if(_frag_color_modifier.dirty())
        *(fragment._output_var) = *(fragment._output_var) + _frag_color_modifier.str();

    if(_vert_main_source.dirty())
    {
        _vert_main_source << "    ";
        vertex._source.insertBefore("gl_Position =", _vert_main_source.str());
    }
}

void PipelineBuildingContext::insertBefore(String& src, const String& statement, const String& str)
{
    String::size_type pos = src.find(statement);
    if(pos != String::npos)
        src.insert(pos, str);
}

}
