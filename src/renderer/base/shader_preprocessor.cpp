#include "renderer/base/shader_preprocessor.h"

#include <regex>

#include "core/types/global.h"

#include "core/base/string_buffer.h"
#include "core/util/strings.h"

#include "renderer/base/gl_context.h"
#include "renderer/base/pipeline_building_context.h"
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

void ShaderPreprocessor::parse(PipelineBuildingContext& context)
{
    parseMainBlock(context);
    parseDeclarations(context);
}

void ShaderPreprocessor::parseMainBlock(PipelineBuildingContext& buildingContext)
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

    _main_block->parse(buildingContext);
}

void ShaderPreprocessor::parseDeclarations(PipelineBuildingContext& context)
{
    _in_declarations.parse(_source, _type == SHADER_TYPE_FRAGMENT ? _IN_OUT_PATTERN : _IN_PATTERN);
    _out_declarations.parse(_source, _OUT_PATTERN);

    _source.search(_UNIFORM_PATTERN, [this](const std::smatch& m)->bool {
        _uniforms.push_back(m[2].str(), m[1].str());
        return true;
    });

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
    for(const Uniform& i : uniforms)
        if(!_uniforms.has(i.name()) && _source.find(i.name()) != String::npos)
        {
            _uniforms.push_back(i.name(), "uniform");

            const String declaration = i.declaration();
            if(declaration.find('[') == String::npos)
                _uniform_declarations << declaration << '\n';
        }
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

void ShaderPreprocessor::CodeBlock::parse(PipelineBuildingContext& buildingContext)
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
            buildingContext.addPredefinedAttribute(Strings::capitalFirst(name), type, 0);
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
