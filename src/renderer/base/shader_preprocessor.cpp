#include "renderer/base/shader_preprocessor.h"

#include <regex>

#include "core/types/global.h"

#include "core/base/string_buffer.h"
#include "core/util/strings.h"

#include "renderer/base/gl_context.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/pipeline_layout.h"

#define VAR_TYPE_PATTERN "\\s+(int|uint8|float|vec2|vec3|vec4|mat3|mat4|sampler2D)\\s+"
#define ATTRIBUTE_PATTERN VAR_TYPE_PATTERN "(?:a_|v_)([\\w\\d_]+);"
#define UNIFORM_PATTERN VAR_TYPE_PATTERN "(u_[\\w\\d_]+);"

namespace ark {

const char* ShaderPreprocessor::ANNOTATION_VERT_IN = "${vert.in}";
const char* ShaderPreprocessor::ANNOTATION_VERT_OUT = "${vert.out}";
const char* ShaderPreprocessor::ANNOTATION_FRAG_IN = "${frag.in}";
const char* ShaderPreprocessor::ANNOTATION_FRAG_OUT = "${frag.out}";
const char* ShaderPreprocessor::ANNOTATION_FRAG_COLOR = "${frag.color}";

std::regex ShaderPreprocessor::_IN_PATTERN("(?:attribute|in)" ATTRIBUTE_PATTERN);
std::regex ShaderPreprocessor::_OUT_PATTERN("(?:varying|out)" ATTRIBUTE_PATTERN);
std::regex ShaderPreprocessor::_IN_OUT_PATTERN("(?:varying|in)" ATTRIBUTE_PATTERN);
std::regex ShaderPreprocessor::_UNIFORM_PATTERN("uniform" UNIFORM_PATTERN);

ShaderPreprocessor::ShaderPreprocessor(ShaderType type, const String& source)
    : _type(type), _source(source), _version(0), _in_declarations(type == SHADER_TYPE_VERTEX ? ANNOTATION_VERT_IN : ANNOTATION_FRAG_IN),
      _out_declarations(type == SHADER_TYPE_VERTEX ? ANNOTATION_VERT_OUT : ANNOTATION_FRAG_OUT)
{
}

void ShaderPreprocessor::addSource(const String& source)
{
    _snippets.emplace_back(SNIPPET_TYPE_SOURCE, source);
}

void ShaderPreprocessor::addModifier(const String& modifier)
{
//    *_output_var = Strings::sprintf("%s * %s", _output_var->c_str(), modifier.c_str());
    _snippets.emplace_back(SNIPPET_TYPE_MULTIPLY, modifier);
}

void ShaderPreprocessor::parse(PipelineBuildingContext& context)
{
    parseMainBlock(context);
    parseDeclarations(context);
}

void ShaderPreprocessor::parseMainBlock(PipelineBuildingContext& buildingContext)
{
    const String source = _source.str();

    if(source.find("void main()") != String::npos)
    {
        DWARN(false, "Shader which contains main function will not be preprocessed by ark shader preprocessor. Try to replace it with \"vec4 ark_main(vec4 position, ...)\" for better flexibilty and compatibilty");
        return;
    }

    static const std::regex FUNC_PATTERN("vec4\\s+ark_main\\(([^)]*)\\)");

    _source = Source();

    source.search(FUNC_PATTERN, [this] (const std::smatch& m)->bool {
        const String prefix = m.prefix().str();
        const String remaining = m.suffix().str();
        String body;
        size_t prefixStart = parseFunctionBody(remaining, body);
        const sp<String> fragment = sp<String>::make();
        _source.append(sp<String>::make(prefix));
        _source.append(fragment);
        _source.append(sp<String>::make(remaining.substr(prefixStart)));
        _main_block = sp<CodeBlock>::make(Function("main", m[1].str(), body.strip()), fragment);
        return false;
    });

    DCHECK(_main_block, "Undefined ark_main in shader");

    _main_block->parse(buildingContext);
}

void ShaderPreprocessor::parseDeclarations(PipelineBuildingContext& context)
{
    _in_declarations.parse(_source, _type == SHADER_TYPE_FRAGMENT ? _IN_OUT_PATTERN : _IN_PATTERN);
    _out_declarations.parse(_source, _OUT_PATTERN);

    _source.replace(_UNIFORM_PATTERN, [this](const std::smatch& m) {
        const sp<String> declaration = sp<String>::make(m.str());
        this->addUniform(m[1].str(), m[2].str(), declaration);
        return declaration;
    });

    if(!_main_block)
        return;

    _main_block->genDefinition();

    {
        StringBuffer sb;

        const String outVar = _type == SHADER_TYPE_VERTEX ? "gl_Position" : ANNOTATION_FRAG_COLOR;
        sb << "\n\nvoid main() {\n    " << outVar << " = ";

        _source.append(sp<String>::make(sb.str()));
        _output_var = sp<String>::make(_main_block->genOutCall(_type));
        _source.append(_output_var);

        _source.append(sp<String>::make(";\n}\n\n"));
    }

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
    return Preprocessor(_type, getDeclarations() + _source.str());
}

void ShaderPreprocessor::insertPredefinedUniforms(const std::vector<Uniform>& uniforms)
{
    for(const Uniform& i : uniforms)
        if(!_uniforms.has(i.name()) && _source.contains(i.name()))
        {
            const String declaration = i.declaration();
            if(declaration.find('[') == String::npos)
                _uniform_declarations.push_back(declaration);
        }
}

Uniform ShaderPreprocessor::getUniformInput(const String& name, Uniform::Type type) const
{
    if(!_uniforms.has(name))
        return Uniform();

    const Declaration& uniform = _uniforms.at(name);
    DCHECK(Uniform::toType(uniform.type()) == type, "Uniform \"%s\" declared type: %s, but it should be %d", name.c_str(), uniform.type().c_str(), type);
    return Uniform(name, type, nullptr, nullptr);
}

size_t ShaderPreprocessor::parseFunctionBody(const String& s, String& body) const
{
    String::size_type pos = s.find('{');
    DCHECK(pos != String::npos, "Cannot parse function body: %s", s.c_str());
    size_t end = Strings::parentheses(s, pos, '{', '}');
    body = s.substr(pos + 1, end - 1).strip();
    return end + 1;
}

void ShaderPreprocessor::addUniform(const String& type, const String& name, const sp<String>& declaration)
{
    Declaration uniform(name, type, declaration);
    if(type.startsWith("sampler"))
        _samplers.push_back(name, std::move(uniform));
    else
        _uniforms.push_back(name, std::move(uniform));
}

void ShaderPreprocessor::addLines(StringBuffer& sb, const std::vector<String>& lines) const
{
    if(lines.size())
    {
        for(const String& i : lines)
            sb << i << '\n';
        sb << '\n';
    }
}

String ShaderPreprocessor::getDeclarations() const
{
    StringBuffer sb;
    if(_version)
        sb << "#version " << _version << '\n';

    sb << '\n';
    addLines(sb, _macro_defines);
    addLines(sb, _uniform_declarations);

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

ShaderPreprocessor::CodeBlock::CodeBlock(const Function& procedure, const sp<String>& placeHolder)
    : _function(procedure), _place_hoder(placeHolder)
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

void ShaderPreprocessor::CodeBlock::genDefinition()
{
    StringBuffer sb;
    sb << "vec4 ark_" << _function._name << "(";
    const auto begin = _function._ins.begin();
    for(auto iter = begin; iter != _function._ins.end(); ++iter)
    {
        if(iter != begin)
            sb << ", ";
        sb << iter->first << " " << iter->second;
    }
    sb << ") {\n    " << _function._body << "\n}";
    *_place_hoder = sb.str();
}

String ShaderPreprocessor::CodeBlock::genOutCall(ShaderPreprocessor::ShaderType type)
{
    StringBuffer sb;
    sb << "ark_main(";
    const auto begin = _function._ins.begin();
    for(auto iter = begin; iter != _function._ins.end(); ++iter)
    {
        if(iter != begin)
            sb << ", ";
        sb << (type == SHADER_TYPE_VERTEX ? "a_" : "v_");
        sb << Strings::capitalFirst(iter->second);
    }
    sb << ')';
    return sb.str();
}

bool ShaderPreprocessor::CodeBlock::hasOutParam(const String& name) const
{
    const String oName = name.startsWith("v_") ? name : String("v_") + name;
    for(const auto& i : _function._outs)
        if(i.second == oName)
            return true;
    return false;
}

ShaderPreprocessor::DeclarationList::DeclarationList(const String& category)
    : _category(category)
{
}

void ShaderPreprocessor::DeclarationList::declare(const String& type, const String& prefix, const String& name)
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

void ShaderPreprocessor::DeclarationList::parse(const Source& src, const std::regex& pattern)
{
    src.search(pattern, [this](const std::smatch& m)->bool {
        _declared[m[2].str()] = m[1].str();
        return true;
    });
}

bool ShaderPreprocessor::DeclarationList::dirty() const
{
    return _lines.dirty();
}

bool ShaderPreprocessor::DeclarationList::has(const String& name) const
{
    return _declared.find(name) != _declared.end();
}

String ShaderPreprocessor::DeclarationList::str() const
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

ShaderPreprocessor::Source::Source(String code)
    : _fragments{sp<String>::make(std::move(code))}
{
}

String ShaderPreprocessor::Source::str() const
{
    StringBuffer sb;
    for(const auto& i : _fragments)
        sb << *i;
    return sb.str();
}

void ShaderPreprocessor::Source::append(const sp<String>& fragment)
{
    _fragments.push_back(fragment);
}

bool ShaderPreprocessor::Source::search(const std::regex& pattern, const std::function<bool (const std::smatch&)>& traveller) const
{
    for(const sp<String>& i : _fragments)
        if(!i->search(pattern, traveller))
            return false;
    return true;
}

bool ShaderPreprocessor::Source::contains(const String& str) const
{
    for(const sp<String>& i : _fragments)
        if(i->find(str) != String::npos)
            return true;
    return false;
}

void ShaderPreprocessor::Source::replace(const std::regex& regexp, const std::function<sp<String>(const std::smatch&)>& replacer)
{
    for(auto iter = _fragments.begin(); iter != _fragments.end(); ++iter)
    {
        const sp<String>& fragment = *iter;
        std::vector<sp<String>> inserting;
        fragment->search(regexp, [&inserting, replacer](const std::smatch& match) {
            inserting.push_back(replacer(match));
            return true;
        }, [&inserting](const String& unmatch) {
            inserting.push_back(sp<String>::make(unmatch));
            return true;
        });

        if(inserting.size() > 1)
        {
            for(const auto& i : inserting)
            {
                iter = _fragments.insert(iter, i);
                ++iter;
            }
            iter = _fragments.erase(iter);
            if(iter == _fragments.end())
                break;
        }
    }
}

void ShaderPreprocessor::Source::insertBefore(const String& statement, const String& str)
{
    for(const sp<String>& i : _fragments)
    {
        String& code = *i;
        String::size_type pos = code.find(statement);
        if(pos != String::npos)
            code.insert(pos, str);
    }
}

ShaderPreprocessor::Declaration::Declaration(const String& name, const String& type, const sp<String>& source)
    : _name(name), _type(type), _source(source)
{
}

const String& ShaderPreprocessor::Declaration::name() const
{
    return _name;
}

const String& ShaderPreprocessor::Declaration::type() const
{
    return _type;
}

}
