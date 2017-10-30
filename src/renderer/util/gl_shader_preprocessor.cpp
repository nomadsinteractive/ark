#include "renderer/util/gl_shader_preprocessor.h"

#include <regex>

#include "core/types/global.h"

#include "core/base/string_builder.h"
#include "core/util/strings.h"

#include "renderer/base/gl_shader_source.h"

namespace ark {

GLShaderPreprocessor::GLShaderPreprocessor(const String& inType, const String& outType)
    : _in_declarations(inType), _out_declarations(outType)
{
}

void GLShaderPreprocessor::parse(GLShaderPreprocessor::Context& context, const String& src, GLShaderSource& shader)
{
    if(src.find("void main()") != String::npos)
    {
        DWARN(false, "Shader which contains main function will not be preprocessed by ark shader preprocessor. Try to replace it with ark_main(vec2 position, ...) for better flexibilty and compatibilty");
        return;
    }

    uint32_t prefixStart = 0;
    static const std::regex FUNC_PATTERN("vec4\\s+ark_main\\(([^)]*)\\)");

    src.search(FUNC_PATTERN, [&prefixStart, this] (const std::smatch& m)->bool {
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

void GLShaderPreprocessor::preprocess(GLShaderPreprocessor::Context& context, String& src, ShaderType type, GLShaderSource& shader)
{
    _in_declarations.parse(src, (context.renderEngine->version() < Ark::OPENGL_30 && type == SHADER_TYPE_FRAGMENT)
                           ? context.renderEngine->outPattern() : context.renderEngine->inPattern());
    _out_declarations.parse(src, context.renderEngine->outPattern());

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

    const String outVar = type == SHADER_TYPE_VERTEX ? "gl_Position" : context.renderEngine->fragmentName();
    sb << "\n\nvoid main() {\n    " << outVar << " = ark_main(";

    const auto begin = _main_block->_procedure._ins.begin();
    for(auto iter = begin; iter != _main_block->_procedure._ins.end(); ++iter)
    {
        if(iter != begin)
            sb << ", ";
        sb << (type == SHADER_TYPE_VERTEX ? "a_" : "v_");
        sb << Strings::capitalFirst(iter->second);
    }
    sb << ");\n}\n\n";

    src = sb.str();

    if(type == SHADER_TYPE_VERTEX)
    {
        for(const auto& i : _main_block->_procedure._ins)
            context.vertexIns.push_back(i);

        for(const std::pair<String, String>& i : _main_block->_procedure._outs)
            context.vertexOuts.push_back(i);
        for(const auto& i : context.vertexIns)
            context.addAttribute(Strings::capitalFirst(i.second), i.first, context.vertexInsDeclared, shader);
    }
    if(type == SHADER_TYPE_FRAGMENT)
    {
        for(const std::pair<String, String>& i : _main_block->_procedure._ins)
            context.fragmentIns.push_back(i);
    }
}

uint32_t GLShaderPreprocessor::parseFunctionBody(const String& s, String& body)
{
    String::size_type pos = s.find('{');
    DCHECK(pos != String::npos, "Cannot parse function body: %s", s.c_str());
    String::size_type end = Strings::parentheses(s, pos, '{', '}');
    body = s.substr(pos + 1, end - 1).strip();
    return end + 1;
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

void GLShaderPreprocessor::Context::addAttribute(const String& name, const String& type, std::map<String, String>& vars, GLShaderSource& source)
{
    if(vars.find(name) == vars.end())
    {
        vars[name] = type;
        source.addAttribute(name, type);
    }
}

//void GLShaderPreprocessor::Context::addAttributeInSource(const String& name, const String& type, std::map<String, String>& vars, std::map<String, String>& varsInSource, GLShaderSource& source)
//{
//    varsInSource[name] = type;
//    addAttribute(name, type, vars, source);
//}

void GLShaderPreprocessor::Context::addVertexSource(const String& source)
{
    _vertex_snippets.push_back(Snippet(SNIPPET_TYPE_SOURCE, source));
}

void GLShaderPreprocessor::Context::addFragmentColorModifier(const String& modifier)
{
    _fragment_snippets.push_back(Snippet(SNIPPET_TYPE_MULTIPLY, modifier));
}

void GLShaderPreprocessor::Context::addFragmentProcedure(const String& name, const List<std::pair<String, String>>& ins, const String& procedure)
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
    _fragment_snippets.push_back(Snippet(SNIPPET_TYPE_PROCEDURE, sb.str()));
    sb.clear();
    sb << "\n    " << renderEngine->fragmentName() << " = ark_" << name << '(' << renderEngine->fragmentName() << callParams.str() << ");";
    _fragment_snippets.push_back(Snippet(SNIPPET_TYPE_PROCEDURE_CALL, sb.str()));
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
