#include "renderer/base/shader_preprocessor.h"

#include <regex>

#include "core/types/global.h"

#include "core/base/string_buffer.h"
#include "core/util/strings.h"

#include "renderer/base/gl_context.h"
#include "renderer/base/pipeline_layout.h"

#define VAR_PATTERN "\\s+(int|uint8|float|vec2|vec3|vec4|mat3|mat4|sampler2D)\\s+(?:a_|v_|u_)([\\w\\d_]+);"

namespace ark {

const char* ShaderPreprocessor::ANNOTATION_VERT_IN = "${vert.in}";
const char* ShaderPreprocessor::ANNOTATION_VERT_OUT = "${vert.out}";
const char* ShaderPreprocessor::ANNOTATION_FRAG_IN = "${frag.in}";
const char* ShaderPreprocessor::ANNOTATION_FRAG_OUT = "${frag.out}";
const char* ShaderPreprocessor::ANNOTATION_FRAG_COLOR = "${frag.color}";

std::regex ShaderPreprocessor::_IN_PATTERN("(?:attribute|in)" VAR_PATTERN);
std::regex ShaderPreprocessor::_OUT_PATTERN("(?:varying|out)" VAR_PATTERN);
std::regex ShaderPreprocessor::_IN_OUT_PATTERN("(?:varying|in)" VAR_PATTERN);
std::regex ShaderPreprocessor::_UNIFORM_PATTERN("uniform" VAR_PATTERN);

ShaderPreprocessor::ShaderPreprocessor(ShaderType type, const String& source)
    : _type(type), _source(source), _in_declarations(type == SHADER_TYPE_VERTEX ? ANNOTATION_VERT_IN : ANNOTATION_FRAG_IN),
      _out_declarations(type == SHADER_TYPE_VERTEX ? ANNOTATION_VERT_OUT : ANNOTATION_FRAG_OUT)
{
}

void ShaderPreprocessor::addSource(const String& source)
{
    _snippets.emplace_back(SNIPPET_TYPE_SOURCE, source);
}

void ShaderPreprocessor::addModifier(const String& modifier)
{
    _snippets.emplace_back(SNIPPET_TYPE_MULTIPLY, modifier);
}

void ShaderPreprocessor::parse(PipelineBuildingContext& context, PipelineLayout& shader)
{
    parseMainBlock(shader);
    parseDeclarations(context);
}

void ShaderPreprocessor::parseMainBlock(PipelineLayout& pipelineLayout)
{
    if(_source.find("void main()") != String::npos)
    {
        DWARN(false, "Shader which contains main function will not be preprocessed by ark shader preprocessor. Try to replace it with \"vec4 ark_main(vec4 position, ...)\" for better flexibilty and compatibilty");
        return;
    }

    uint32_t prefixStart = 0;
    static const std::regex FUNC_PATTERN("vec4\\s+ark_main\\(([^)]*)\\)");

    _source.search(FUNC_PATTERN, [&prefixStart, this] (const std::smatch& m)->bool {
        const String prefix = prefixStart == 0 ? m.prefix().str() : m.prefix().str().substr(prefixStart);
        const String remaining = m.suffix().str();
        String body;
        prefixStart = parseFunctionBody(remaining, body);
        _main_block = sp<CodeBlock>::make(prefix, Function("main", m[1].str(), body.strip()), remaining.substr(prefixStart));
        return false;
    });

    DCHECK(_main_block, "Undefined ark_main in shader");

    _main_block->parse(pipelineLayout);
}

void ShaderPreprocessor::parseDeclarations(PipelineBuildingContext& context)
{
    _in_declarations.parse(_source, _type == SHADER_TYPE_FRAGMENT ? _IN_OUT_PATTERN : _IN_PATTERN);
    _out_declarations.parse(_source, _OUT_PATTERN);

    if(!_main_block)
        return;

    StringBuffer sb;
    {
        sb << _main_block->_prefix;
        sb << "vec4 ark_" << _main_block->_function._name << "(";
        const auto begin = _main_block->_function._ins.begin();
        for(auto iter = begin; iter != _main_block->_function._ins.end(); ++iter)
        {
            if(iter != begin)
                sb << ", ";
            sb << iter->first << " " << iter->second;
        }
        sb << ") {\n    " << _main_block->_function._body << "\n}" << _main_block->_suffix;
    }

    const String outVar = _type == SHADER_TYPE_VERTEX ? "gl_Position" : ANNOTATION_FRAG_COLOR;
    sb << "\n\nvoid main() {\n    " << outVar << " = ark_main(";

    const auto begin = _main_block->_function._ins.begin();
    for(auto iter = begin; iter != _main_block->_function._ins.end(); ++iter)
    {
        if(iter != begin)
            sb << ", ";
        sb << (_type == SHADER_TYPE_VERTEX ? "a_" : "v_");
        sb << Strings::capitalFirst(iter->second);
    }
    sb << ");\n}\n\n";

    _source = sb.str();

    if(_type == SHADER_TYPE_VERTEX)
    {
        for(const auto& i : _main_block->_function._ins)
            context._vertex_in.push_back(i);

        for(const std::pair<String, String>& i : _main_block->_function._outs)
            context._vertex_out.push_back(i);

        for(const auto& i : context._vertex_in)
        {
            const String name = Strings::capitalFirst(i.second);
            const String type = i.first;
            if(context._vert_in_declared.find(name) == context._vert_in_declared.end())
            {
                context._vert_in_declared[name] = type;
                context.addAttribute(name, type);
            }
        }
    }
    else if(_type == SHADER_TYPE_FRAGMENT)
    {
        for(const std::pair<String, String>& i : _main_block->_function._ins)
            context._fragment_in.push_back(i);
    }
}

ShaderPreprocessor::Preprocessor ShaderPreprocessor::preprocess()
{
    _source = getDeclarations() + _source;
    return Preprocessor(_type, _source);
}

void ShaderPreprocessor::insertPredefinedUniforms(const std::vector<Uniform>& uniforms)
{
//    static const std::regex UNIFORM_PATTERN("uniform\\s+\\w+\\s+(\\w+)(?:\\[\\d+\\])?;");
    std::set<String> names;
    std::vector<String> generated;
    _source.search(_UNIFORM_PATTERN, [this, &names](const std::smatch& m)->bool {
        names.insert(m[1].str());
        _uniforms.push_back(std::make_pair(m[0].str(), m[1].str()));
        return true;
    });

    for(const Uniform& i : uniforms)
        if(names.find(i.name()) == names.end() && _source.find(i.name()) != String::npos)
        {
            generated.push_back(i.declaration());
            _uniforms.push_back(std::make_pair("uniform", i.name()));
        }

    for(const String& i : generated)
        if(i.find('[') == String::npos)
            _uniform_declarations << i << '\n';
}

uint32_t ShaderPreprocessor::parseFunctionBody(const String& s, String& body) const
{
    String::size_type pos = s.find('{');
    DCHECK(pos != String::npos, "Cannot parse function body: %s", s.c_str());
    String::size_type end = Strings::parentheses(s, pos, '{', '}');
    body = s.substr(pos + 1, end - 1).strip();
    return end + 1;
}

void ShaderPreprocessor::insertAfter(const String& statement, const String& str)
{
    String::size_type pos = _source.find(statement);
    if(pos != String::npos)
        _source.insert(pos + 1, str);
}

String ShaderPreprocessor::getDeclarations() const
{
    StringBuffer sb;
    if(_uniform_declarations.dirty())
        sb << '\n' << _uniform_declarations.str();
    if(_in_declarations.dirty())
        sb << '\n' << _in_declarations.str();
    const String s = _out_declarations.str();
    if(_out_declarations.dirty())
        sb << '\n' << _out_declarations.str();
    return sb.str();
}

ShaderPreprocessor::Snippet::Snippet(ShaderPreprocessor::SnippetType type, const String& src)
    : _type(type), _src(src)
{
}

ShaderPreprocessor::Function::Function(const String& name, const String& params, const String& body)
    : _name(name), _params(params), _body(body)
{
}

ShaderPreprocessor::CodeBlock::CodeBlock(const String& prefix, const ShaderPreprocessor::Function& procedure, const String& suffix)
    : _prefix(prefix), _function(procedure), _suffix(suffix)
{
}

void ShaderPreprocessor::CodeBlock::parse(PipelineLayout& pipelineLayout)
{
    for(const String& i : _function._params.split(','))
    {
        String s = i.strip();
        bool out = false;
        if(s.startsWith("out "))
        {
            s = s.substr(4).strip();
            out = true;
        }
        else if(s.startsWith("in "))
            s = s.substr(3).strip();
        String type, name;
        Strings::cut(s, type, name, ' ');
        DCHECK(type && name, "Cannot parse function arguments: %s", s.c_str());
        if(out)
            _function._outs.push_back(std::pair<String, String>(type, name));
        else
        {
            _function._ins.push_back(std::pair<String, String>(type, name));
            pipelineLayout.addAttribute(Strings::capitalFirst(name), type);
        }
    }
}

bool ShaderPreprocessor::CodeBlock::hasOutParam(const String& name) const
{
    const String oName = name.startsWith("v_") ? name : String("v_") + name;
    for(const auto& i : _function._outs)
        if(i.second == oName)
            return true;
    return false;
}

PipelineBuildingContext::PipelineBuildingContext(PipelineLayout& pipelineLayout, const String& vertex, const String& fragment)
    : _pipeline_layout(pipelineLayout), _vertex(ShaderPreprocessor::SHADER_TYPE_VERTEX, vertex), _fragment(ShaderPreprocessor::SHADER_TYPE_FRAGMENT, fragment)
{
}

void PipelineBuildingContext::initialize()
{
    _vertex.parse(*this, _pipeline_layout);
    _fragment.parse(*this, _pipeline_layout);

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
    if(_pipeline_layout._snippet)
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

    for(auto iter : _pipeline_layout._input->streams())
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
    _pipeline_layout._input->addAttribute(name, attr);
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


ShaderPreprocessor::Declaration::Declaration(const String& category)
    : _category(category)
{
}

void ShaderPreprocessor::Declaration::declare(const String& type, const String& prefix, const String& name)
{
    const auto iter = _declared.find(name);
    if(iter == _declared.end())
    {
        _lines << _category << ' ' << type << ' ' << prefix << name << ";\n";
        _declared[name] = type;
    }
    else
        DCHECK(iter->second == type, "Declared type \"\" and variable type \"\" mismatch", iter->second.c_str(), type.c_str());
}

void ShaderPreprocessor::Declaration::parse(const String& src, const std::regex& pattern)
{
    src.search(pattern, [this](const std::smatch& m)->bool {
        _declared[m[2].str()] = m[1].str();
        return true;
    });
}

bool ShaderPreprocessor::Declaration::dirty() const
{
    return _lines.dirty();
}

bool ShaderPreprocessor::Declaration::has(const String& name) const
{
    return _declared.find(name) != _declared.end();
}

String ShaderPreprocessor::Declaration::str() const
{
    return _lines.str();
}

ShaderPreprocessor::Preprocessor::Preprocessor()
    : _type(SHADER_TYPE_NONE)
{
}

ShaderPreprocessor::Preprocessor::Preprocessor(ShaderPreprocessor::ShaderType type, String source)
    : _type(type), _source(std::move(source))
{
}

String ShaderPreprocessor::Preprocessor::process(const GLContext& glContext) const
{
    DCHECK(glContext.version() > 0, "Unintialized GLContext");

    static std::regex var_pattern("\\$\\{([\\w.]+)\\}");

    StringBuffer sb;
    if(_type == SHADER_TYPE_FRAGMENT && glContext.version() >= Ark::OPENGL_30)
    {
        sb << "#define texture2D texture\n";
        sb << "#define textureCube texture\n";
        sb << "";
    }

    const std::map<String, String>& annotations = glContext.annotations();

    sb << _source.replace(var_pattern, [&annotations] (Array<String>& matches)->String {
        const String& varName = matches.buf()[1];
        const auto iter = annotations.find(varName);
        DCHECK(iter != annotations.end(), "Cannot find constant \"%s\" in RenderEngine", varName.c_str());
        return iter->second;
    });

    return sb.str();
}

}