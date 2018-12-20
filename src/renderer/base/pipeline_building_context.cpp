#include "renderer/base/pipeline_building_context.h"

#include <regex>

#include "core/base/string_buffer.h"
#include "core/util/strings.h"

#include "renderer/base/gl_context.h"
#include "renderer/base/pipeline_layout.h"

namespace ark {

PipelineBuildingContext::PipelineBuildingContext(PipelineLayout& pipelineLayout, const String& vertex, const String& fragment)
    : _pipeline_layout(pipelineLayout), _input(pipelineLayout.input()), _vertex(ShaderPreprocessor::SHADER_TYPE_VERTEX, vertex), _fragment(ShaderPreprocessor::SHADER_TYPE_FRAGMENT, fragment)
{
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
//    if(_pipeline_layout._snippet)
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

void PipelineBuildingContext::addFragmentProcedure(const String& name, const List<std::pair<String, String>>& ins, const String& procedure)
{
    StringBuffer declareParams;
    StringBuffer callParams;
    for(const auto& i : ins)
    {
        declareParams << ", ";
        declareParams << i.first << ' ' << i.second;

        callParams << ", ";
        callParams << "a_" << Strings::capitalFirst(i.second);
    }
    StringBuffer sb;
    sb << "vec4 ark_" << name << "(vec4 c" << declareParams.str() << ") {\n    " << procedure << "\n}\n\n";
    _fragment._snippets.emplace_back(ShaderPreprocessor::SNIPPET_TYPE_PROCEDURE, sb.str());
    sb.clear();
    sb << "\n    " << ShaderPreprocessor::ANNOTATION_FRAG_COLOR << " = ark_" << name << '(' << ShaderPreprocessor::ANNOTATION_FRAG_COLOR << callParams.str() << ");";
    _fragment._snippets.emplace_back(ShaderPreprocessor::SNIPPET_TYPE_PROCEDURE_CALL, sb.str());
}

void PipelineBuildingContext::preCompile()
{
    doSnippetPrecompile();
    doPrecompile(_vertex._source, _fragment._source);
}

void PipelineBuildingContext::addAttribute(const String& name, const String& type)
{
    Attribute& attr = addPredefinedAttribute(name, type, 0);
    _input->addAttribute(name, attr);
}

void PipelineBuildingContext::addUniform(const String& name, Uniform::Type type, const sp<Flatable>& flatable, const sp<Changed>& changed)
{
    _uniforms.push_back(name, Uniform(name, type, flatable, changed));
    _pipeline_layout.addUniform(name, type, flatable, changed);
}

Attribute& PipelineBuildingContext::addPredefinedAttribute(const String& name, const String& type, uint32_t scopes)
{
    if(_attributes.find(name) == _attributes.end())
        _attributes[name] = makePredefinedAttribute(name, type);

    if(scopes == ShaderPreprocessor::SHADER_TYPE_FRAGMENT)
        _fragment_in.push_back(std::pair<String, String>(type, name));

    return _attributes[name];
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
        case ShaderPreprocessor::SNIPPET_TYPE_PROCEDURE:
            _frag_procedures << i._src;
            break;
        case ShaderPreprocessor::SNIPPET_TYPE_PROCEDURE_CALL:
            _frag_procedure_calls << i._src;
            break;
        default:
            break;
        }
    }
}

void PipelineBuildingContext::doPrecompile(String& vertSource, String& fragSource)
{
    if(_frag_color_modifier.dirty())
    {
        String::size_type pos = fragSource.rfind(';');
        DCHECK(pos != String::npos, "Cannot find fragment color modifier point, empty fragment shader?");
        fragSource.insert(pos, _frag_color_modifier.str());
    }

    if(_frag_procedures.dirty())
        insertBefore(fragSource, "void main()", _frag_procedures.str());

    if(_frag_procedure_calls.dirty())
    {
        String::size_type pos = fragSource.rfind(';');
        DCHECK(pos != String::npos, "Cannot find fragment color modifier point, empty fragment shader?");
        fragSource.insert(pos + 1, _frag_procedure_calls.str());
    }

    if(_vert_main_source.dirty())
    {
        _vert_main_source << "    ";
        insertBefore(vertSource, "gl_Position =", _vert_main_source.str());
    }
}

void PipelineBuildingContext::insertBefore(String& src, const String& statement, const String& str)
{
    String::size_type pos = src.find(statement);
    if(pos != String::npos)
        src.insert(pos, str);
}

}
