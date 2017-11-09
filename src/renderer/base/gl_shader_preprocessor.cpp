#include "renderer/base/gl_shader_preprocessor.h"

#include <regex>

#include "core/types/global.h"

#include "core/base/string_builder.h"
#include "core/util/strings.h"

#include "renderer/base/gl_context.h"
#include "renderer/base/gl_shader_source.h"

#define VAR_PATTERN "\\s+([\\w\\d]+)\\s+(?:a_|v_)([\\w\\d_]+);"

namespace ark {

const char* GLShaderPreprocessor::ANNOTATION_VERT_IN = "${vert.in}";
const char* GLShaderPreprocessor::ANNOTATION_VERT_OUT = "${vert.out}";
const char* GLShaderPreprocessor::ANNOTATION_FRAG_IN = "${frag.in}";
const char* GLShaderPreprocessor::ANNOTATION_FRAG_OUT = "${frag.out}";
const char* GLShaderPreprocessor::ANNOTATION_FRAG_COLOR = "${frag.color}";

std::regex GLShaderPreprocessor::_IN_PATTERN("(?:attribute|in)" VAR_PATTERN);
std::regex GLShaderPreprocessor::_OUT_PATTERN("(?:varying|out)" VAR_PATTERN);
std::regex GLShaderPreprocessor::_IN_OUT_PATTERN("(?:varying|in)" VAR_PATTERN);

GLShaderPreprocessor::GLShaderPreprocessor(ShaderType type, const String& source)
    : _type(type), _source(source), _in_declarations(type == SHADER_TYPE_VERTEX ? ANNOTATION_VERT_IN : ANNOTATION_FRAG_IN),
      _out_declarations(type == SHADER_TYPE_VERTEX ? ANNOTATION_VERT_OUT : ANNOTATION_FRAG_OUT)
{
}

void GLShaderPreprocessor::parseMainFunction(GLShaderSource& shader)
{
    if(_source.find("void main()") != String::npos)
    {
        DWARN(false, "Shader which contains main function will not be preprocessed by ark shader preprocessor. Try to replace it with \"vec4 ark_main(vec2 position, ...)\" for better flexibilty and compatibilty");
        return;
    }

    uint32_t prefixStart = 0;
    static const std::regex FUNC_PATTERN("vec4\\s+ark_main\\(([^)]*)\\)");

    _source.search(FUNC_PATTERN, [&prefixStart, this] (const std::smatch& m)->bool {
        const String prefix = prefixStart == 0 ? m.prefix().str() : m.prefix().str().substr(prefixStart);
        const String remaining = m.suffix().str();
        String body;
        prefixStart = parseFunctionBody(remaining, body);
        _main_block = sp<CodeBlock>::make(prefix, Procedure("main", m[1].str(), body.strip()), remaining.substr(prefixStart));
        return false;
    });

    DCHECK(_main_block, "Undefined ark_main in shader");

    parseCodeBlock(_main_block, shader);
}

void GLShaderPreprocessor::parseDeclarations(GLShaderPreprocessorContext& context, GLShaderSource& shader)
{
    _in_declarations.parse(_source, _type == SHADER_TYPE_FRAGMENT ? _IN_OUT_PATTERN : _IN_PATTERN);
    _out_declarations.parse(_source, _OUT_PATTERN);

    if(!_main_block)
        return;

    StringBuilder sb;
    {
        sb << _main_block->_prefix;
        sb << "vec4 ark_" << _main_block->_procedure._name << "(";
        const auto begin = _main_block->_procedure._ins.begin();
        for(auto iter = begin; iter != _main_block->_procedure._ins.end(); ++iter)
        {
            if(iter != begin)
                sb << ", ";
            sb << iter->first << " " << iter->second;
        }
        sb << ") {\n    " << _main_block->_procedure._body << "\n}" << _main_block->_suffix;
    }

    const String outVar = _type == SHADER_TYPE_VERTEX ? "gl_Position" : ANNOTATION_FRAG_COLOR;
    sb << "\n\nvoid main() {\n    " << outVar << " = ark_main(";

    const auto begin = _main_block->_procedure._ins.begin();
    for(auto iter = begin; iter != _main_block->_procedure._ins.end(); ++iter)
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
        for(const auto& i : _main_block->_procedure._ins)
            context._vertex_in.push_back(i);

        for(const std::pair<String, String>& i : _main_block->_procedure._outs)
            context._vertex_out.push_back(i);

        for(const auto& i : context._vertex_in)
            context.addAttribute(Strings::capitalFirst(i.second), i.first, context._vert_in_declared, shader);
    }
    else if(_type == SHADER_TYPE_FRAGMENT)
    {
        for(const std::pair<String, String>& i : _main_block->_procedure._ins)
            context._fragment_in.push_back(i);
    }
}

void GLShaderPreprocessor::preprocess()
{
    insertAfter(";", getDeclarations());
}

String GLShaderPreprocessor::process(const GLContext& glContext) const
{
    DCHECK(glContext.version() > 0, "Unintialized GLContext");

    static std::regex var_pattern("\\$\\{([\\w.]+)\\}");

    StringBuilder sb;
    if(_type == SHADER_TYPE_FRAGMENT && glContext.version() >= Ark::OPENGL_30)
    {
        sb << "#define texture2D texture\n";
        sb << "";
    }

    std::map<String, String> annotations = glContext.annotations();
    annotations.insert(_annotations.begin(), _annotations.end());

    sb << _source.replace(var_pattern, [&annotations] (Array<String>& matches)->String {
        const String& varName = matches.array()[1];
        const auto iter = annotations.find(varName);
        DCHECK(iter != annotations.end(), "Cannot find constant \"%s\" in RenderEngine", varName.c_str());
        return iter->second;
    });

    return sb.str();
}

void GLShaderPreprocessor::insertPredefinedUniforms(const List<GLUniform>& uniforms)
{
    static const std::regex UNIFORM_PATTERN("uniform\\s+[\\w\\d]+\\s+([^;]+);");
    std::set<String> names;
    List<String> generated;
    _source.search(UNIFORM_PATTERN, [&names](const std::smatch& m)->bool {
        names.insert(m[1].str());
        return true;
    });

    for(const GLUniform& i : uniforms)
        if(names.find(i.name()) == names.end() && _source.find(i.name()) != String::npos)
            generated.push_back(i.declaration());

    for(const String& i : generated)
        _uniform_declarations << i << '\n';
}

uint32_t GLShaderPreprocessor::parseFunctionBody(const String& s, String& body)
{
    String::size_type pos = s.find('{');
    DCHECK(pos != String::npos, "Cannot parse function body: %s", s.c_str());
    String::size_type end = Strings::parentheses(s, pos, '{', '}');
    body = s.substr(pos + 1, end - 1).strip();
    return end + 1;
}

void GLShaderPreprocessor::insertAfter(const String& statement, const String& str)
{
    String::size_type pos = _source.find(statement);
    if(pos != String::npos)
        _source.insert(pos + 1, str);
}

String GLShaderPreprocessor::getDeclarations()
{
    StringBuilder sb;
    if(_uniform_declarations.dirty())
        sb << '\n' << _uniform_declarations.str();
    if(_in_declarations.dirty())
        sb << '\n' << _in_declarations.str();
    const String s = _out_declarations.str();
    if(_out_declarations.dirty())
        sb << '\n' << _out_declarations.str();
    return sb.str();
}

void GLShaderPreprocessor::declare(StringBuilder& sb, const List<std::pair<String, String>>& vars, const String& inType, const String& prefix, std::map<String, String>& declared) const
{
    for(const auto& i : vars)
    {
        const String name = Strings::capitalFirst(i.second);
        const String& type = i.first;
        const auto iter = declared.find(name);
        if(iter != declared.end())
        {
            DCHECK(iter->second == type, "Declared type \"\" and variable type \"\" mismatch", iter->second.c_str(), type.c_str());
            return;
        }
        sb << inType << " " << type << " " << prefix << name << ";\n";
        declared[name] = type;
    }
}

void GLShaderPreprocessor::parseCodeBlock(CodeBlock& codeBlock, GLShaderSource& shader)
{
    const std::vector<String> params = codeBlock._procedure._params.split(',');
    for(auto iter = params.begin(); iter != params.end(); ++iter)
    {
        String s = (*iter).strip();
        bool out = false;
        if(s.startsWith("out "))
        {
            s = s.substr(4).strip();
            out = true;
        }
        String type, name;
        Strings::cut(s, type, name, ' ');
        DCHECK(type && name, "Cannot parse function arguments: %s", s.c_str());
        if(out)
            codeBlock._procedure._outs.push_back(std::pair<String, String>(type, name));
        else
        {
            codeBlock._procedure._ins.push_back(std::pair<String, String>(type, name));
            shader.addPredefinedAttribute(Strings::capitalFirst(name), type);
        }
    }
}

GLShaderPreprocessor::Snippet::Snippet(GLShaderPreprocessor::SnippetType type, const String& src)
    : _type(type), _src(src)
{
}

GLShaderPreprocessor::Snippet::Snippet(const GLShaderPreprocessor::Snippet& other)
    : _type(other._type), _src(other._src)
{
}

GLShaderPreprocessor::Snippet::Snippet(GLShaderPreprocessor::Snippet&& other)
    : _type(other._type), _src(std::move(other._src))
{
}

GLShaderPreprocessor::Procedure::Procedure(const String& name, const String& params, const String& body)
    : _name(name), _params(params), _body(body)
{
}

GLShaderPreprocessor::Procedure::Procedure(const GLShaderPreprocessor::Procedure& other)
    : _name(other._name), _params(other._params), _body(other._body)
{
}

GLShaderPreprocessor::Procedure::Procedure(GLShaderPreprocessor::Procedure&& other)
    : _name(std::move(other._name)), _params(std::move(other._params)), _body(std::move(other._body))
{
}

GLShaderPreprocessor::CodeBlock::CodeBlock(const String& prefix, const GLShaderPreprocessor::Procedure& procedure, const String& suffix)
    : _prefix(prefix), _procedure(procedure), _suffix(suffix)
{
}

GLShaderPreprocessor::CodeBlock::CodeBlock(const GLShaderPreprocessor::CodeBlock& other)
    : _prefix(other._prefix), _procedure(other._procedure), _suffix(other._suffix)
{
}

GLShaderPreprocessor::CodeBlock::CodeBlock(GLShaderPreprocessor::CodeBlock&& other)
    : _prefix(std::move(other._prefix)), _procedure(std::move(other._procedure)), _suffix(std::move(other._suffix))
{
}

void GLShaderPreprocessorContext::addAttribute(const String& name, const String& type, std::map<String, String>& vars, GLShaderSource& source)
{
    if(vars.find(name) == vars.end())
    {
        vars[name] = type;
        source.addAttribute(name, type);
    }
}

void GLShaderPreprocessorContext::addVertexSource(const String& source)
{
    _vert_snippets.push_back(GLShaderPreprocessor::Snippet(GLShaderPreprocessor::SNIPPET_TYPE_SOURCE, source));
}

void GLShaderPreprocessorContext::addFragmentColorModifier(const String& modifier)
{
    _frag_snippets.push_back(GLShaderPreprocessor::Snippet(GLShaderPreprocessor::SNIPPET_TYPE_MULTIPLY, modifier));
}

void GLShaderPreprocessorContext::addFragmentProcedure(const String& name, const List<std::pair<String, String>>& ins, const String& procedure)
{
    StringBuilder declareParams;
    StringBuilder callParams;
    for(const auto& i : ins)
    {
        declareParams << ", ";
        declareParams << i.first << ' ' << i.second;

        callParams << ", ";
        callParams << "a_" << Strings::capitalFirst(i.second);
    }
    StringBuilder sb;
    sb << "vec4 ark_" << name << "(vec4 c" << declareParams.str() << ") {\n    " << procedure << "\n}\n\n";
    _frag_snippets.push_back(GLShaderPreprocessor::Snippet(GLShaderPreprocessor::SNIPPET_TYPE_PROCEDURE, sb.str()));
    sb.clear();
    sb << "\n    " << GLShaderPreprocessor::ANNOTATION_FRAG_COLOR << " = ark_" << name << '(' << GLShaderPreprocessor::ANNOTATION_FRAG_COLOR << callParams.str() << ");";
    _frag_snippets.push_back(GLShaderPreprocessor::Snippet(GLShaderPreprocessor::SNIPPET_TYPE_PROCEDURE_CALL, sb.str()));
}

void GLShaderPreprocessorContext::precompile(String& vertSource, String& fragSource)
{
    doSnippetPrecompile();
    doPrecompile(vertSource, fragSource);
}

void GLShaderPreprocessorContext::doSnippetPrecompile()
{
    for(const GLShaderPreprocessor::Snippet& i : _vert_snippets)
    {
        switch(i._type)
        {
        case GLShaderPreprocessor::SNIPPET_TYPE_SOURCE:
            _vert_main_source << i._src << '\n';
            break;
        default:
            break;
        }
    }

    for(const GLShaderPreprocessor::Snippet& i : _frag_snippets)
    {
        switch(i._type)
        {
        case GLShaderPreprocessor::SNIPPET_TYPE_MULTIPLY:
            _frag_color_modifier << " * " << i._src;
            break;
        case GLShaderPreprocessor::SNIPPET_TYPE_PROCEDURE:
            _frag_procedures << i._src;
            break;
        case GLShaderPreprocessor::SNIPPET_TYPE_PROCEDURE_CALL:
            _frag_procedure_calls << i._src;
            break;
        default:
            break;
        }
    }
}

void GLShaderPreprocessorContext::doPrecompile(String& vertSource, String& fragSource)
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

void GLShaderPreprocessorContext::insertBefore(String& src, const String& statement, const String& str)
{
    String::size_type pos = src.find(statement);
    if(pos != String::npos)
        src.insert(pos, str);
}


GLShaderPreprocessor::Declaration::Declaration(const String& category)
    : _category(category)
{
}

void GLShaderPreprocessor::Declaration::declare(const String& type, const String& prefix, const String& name)
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

void GLShaderPreprocessor::Declaration::parse(const String& src, const std::regex& pattern)
{
    src.search(pattern, [this](const std::smatch& m)->bool {
        _declared[m[2].str()] = m[1].str();
        return true;
    });
}

bool GLShaderPreprocessor::Declaration::dirty() const
{
    return _lines.dirty();
}

bool GLShaderPreprocessor::Declaration::has(const String& name) const
{
    return _declared.find(name) != _declared.end();
}

String GLShaderPreprocessor::Declaration::str() const
{
    return _lines.str();
}

}
