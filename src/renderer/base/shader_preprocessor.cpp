#include "renderer/base/shader_preprocessor.h"

#include <regex>

#include "core/types/global.h"

#include "core/base/string_buffer.h"
#include "core/base/string_table.h"
#include "core/types/global.h"
#include "core/util/strings.h"

#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/render_context.h"

#define VAR_TYPE_PATTERN    "\\s+(int|uint8|float|vec2|vec3|vec4|mat3|mat4|sampler2D|samplerCube)\\s+"
#define ATTRIBUTE_PATTERN   VAR_TYPE_PATTERN "(?:a_|v_)(\\w+);"
#define UNIFORM_PATTERN     "\\s+(\\w+)\\s+" "(u_\\w+)(?:\\[\\s*(\\d+)\\s*\\])?;"

#define INDENT_STR "    "


namespace ark {

const char* ShaderPreprocessor::ANNOTATION_VERT_IN = "${vert.in}";
const char* ShaderPreprocessor::ANNOTATION_VERT_OUT = "${vert.out}";
const char* ShaderPreprocessor::ANNOTATION_FRAG_IN = "${frag.in}";
const char* ShaderPreprocessor::ANNOTATION_FRAG_OUT = "${frag.out}";
const char* ShaderPreprocessor::ANNOTATION_FRAG_COLOR = "${frag.color}";

static std::regex _INCLUDE_PATTERN("#include\\s*[<\"]([^>\"]+)[>\"]");
static std::regex _STRUCT_PATTERN("struct\\s+(\\w+)\\s*\\{[^}]+\\}\\s*;");
static std::regex _IN_PATTERN("(?:attribute|in)" ATTRIBUTE_PATTERN);
static std::regex _OUT_PATTERN("(?:varying|out)" ATTRIBUTE_PATTERN);
static std::regex _IN_OUT_PATTERN("(?:varying|in)" ATTRIBUTE_PATTERN);
static std::regex _UNIFORM_PATTERN("uniform" UNIFORM_PATTERN);

ShaderPreprocessor::ShaderPreprocessor(ShaderType type)
    : _type(type), _version(0), _ins(_attribute_declarations, type == SHADER_TYPE_VERTEX ? ANNOTATION_VERT_IN : ANNOTATION_FRAG_IN),
      _outs(_attribute_declarations, type == SHADER_TYPE_VERTEX ? ANNOTATION_VERT_OUT : ANNOTATION_FRAG_OUT),
      _uniforms(_uniform_declarations, "uniform"), _samplers(_uniform_declarations, "uniform"), _pre_main(sp<String>::make()),
      _output_var(sp<String>::make()), _post_main(sp<String>::make())
{
}

void ShaderPreprocessor::addPreMainSource(const String& source)
{
    *_pre_main = Strings::sprintf(INDENT_STR "%s\n%s", source.c_str(), _pre_main->c_str());
}

void ShaderPreprocessor::addPostMainSource(const String& source)
{
    *_post_main = Strings::sprintf("%s\n" INDENT_STR "%s", _post_main->c_str(), source.c_str());
}

void ShaderPreprocessor::addModifier(const String& modifier)
{
    *_output_var = Strings::sprintf("%s * %s", _output_var->c_str(), modifier.c_str());
}

void ShaderPreprocessor::initialize(const String& source, PipelineBuildingContext& context)
{
    parseMainBlock(source, context);
    parseDeclarations(context);
}

void ShaderPreprocessor::parseMainBlock(const String& source, PipelineBuildingContext& buildingContext)
{
    if(source.find("void main()") != String::npos)
    {
        DWARN(false, "Shader which contains main function will not be preprocessed by ark shader preprocessor. Try to replace it with \"vec4 ark_main(vec4 position, ...)\" for better flexibilty and compatibilty");
        return;
    }

    static const std::regex FUNC_PATTERN("vec4\\s+ark_main\\(([^)]*)\\)");

    source.search(FUNC_PATTERN, [this] (const std::smatch& m)->bool {
        const String prefix = m.prefix().str();
        const String remaining = m.suffix().str();
        String body;
        size_t prefixStart = parseFunctionBody(remaining, body);
        const sp<String> fragment = sp<String>::make();
        _main.push_back(sp<String>::make(prefix));
        _main.push_back(fragment);
        _main.push_back(sp<String>::make(remaining.substr(prefixStart)));
        _main_block = sp<CodeBlock>::make(Function("main", m[1].str(), body.strip()), fragment);
        return false;
    });

    DCHECK(_main_block, "Undefined ark_main in shader");

    _main_block->parse(buildingContext);
}

void ShaderPreprocessor::parseDeclarations(PipelineBuildingContext& context)
{
    _ins.parse(_type == SHADER_TYPE_FRAGMENT ? _IN_OUT_PATTERN : _IN_PATTERN);
    _outs.parse(_OUT_PATTERN);

    _main.replace(_STRUCT_PATTERN, [this](const std::smatch& m) {
        const sp<String> declaration = sp<String>::make(m.str());
        this->_struct_declarations.push_back(declaration);
        return nullptr;
    });

    _main.replace(_INCLUDE_PATTERN, [this](const std::smatch& m) {
        const String filepath = m.str();
        this->addInclude(m.str(), m[1].str());
        return nullptr;
    });

    _main.replace(_UNIFORM_PATTERN, [this](const std::smatch& m) {
        const sp<String> declaration = sp<String>::make(m.str());
        this->addUniform(m[1].str(), m[2].str(), declaration);
        return nullptr;
    });

    if(!_main_block)
        return;

    _main_block->genDefinition();

    {
        const String outVar = outputName();

        _main.push_back(sp<String>::make("\n\nvoid main() {\n"));
        _main.push_back(_pre_main);
        _main.push_back(sp<String>::make(Strings::sprintf(INDENT_STR "%s = ", outVar.c_str())));
        *_output_var = _main_block->genOutCall(_type);
        _main.push_back(_output_var);
        _main.push_back(sp<String>::make(";"));
        _main.push_back(_post_main);

        _main.push_back(sp<String>::make("\n}\n\n"));
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
    return Preprocessor(_type, getDeclarations() + _main.str());
}

void ShaderPreprocessor::setupBindings(const std::vector<sp<Uniform>>& uniforms, int32_t& binding)
{
    int32_t next = binding;
    for(const sp<Uniform>& i : uniforms)
    {
        String::size_type pos = i->name().find('[');
        DCHECK(pos != 0, "Illegal uniform name: %s", i->name().c_str());
        if(_main.contains(pos == String::npos ? i->name() : i->name().substr(0, pos)))
        {
            if(i->binding() == -1)
            {
                next = binding + 1;
                i->setBinding(binding);
            }

            if(!_uniforms.has(i->name()))
            {
                const String type = i->getDeclaredType();
                sp<String> declaration = sp<String>::make(i->declaration("uniform "));
                _uniforms.vars().push_back(i->name(), Declaration(i->name(), type, declaration));
                if(pos == String::npos)
                    _uniform_declarations.push_back(std::move(declaration));
            }
        }
    }
    binding = next;
}

sp<Uniform> ShaderPreprocessor::getUniformInput(const String& name, Uniform::Type type) const
{
    if(!_uniforms.has(name))
        return nullptr;

    const Declaration& declaration = _uniforms.vars().at(name);
    DCHECK(Uniform::toType(declaration.type()) == type, "Uniform \"%s\" declared type: %s, but it should be %d", name.c_str(), declaration.type().c_str(), type);
    return sp<Uniform>::make(name, type, nullptr, nullptr);
}

String ShaderPreprocessor::outputName() const
{
    return _type == SHADER_TYPE_VERTEX ? "gl_Position" : ANNOTATION_FRAG_COLOR;
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
        _samplers.vars().push_back(name, std::move(uniform));
    else
        _uniforms.vars().push_back(name, std::move(uniform));
    _uniform_declarations.push_back(declaration);
}

String ShaderPreprocessor::getDeclarations() const
{
    StringBuffer sb;
    if(_version && !_main.contains("#version "))
        sb << "#version " << _version << '\n';

    sb << '\n';
    if(_macro_defines.size())
    {
        for(const String& i : _macro_defines)
            sb << i << '\n';
        sb << '\n';
    }

    sb << _includes.str('\n');
    sb << _struct_declarations.str('\n');
    sb << _uniform_declarations.str('\n');
    sb << _attribute_declarations.str('\n');
    return sb.str();
}

void ShaderPreprocessor::addInclude(const String& source, const String& filepath)
{
    const Global<StringTable> stringtable;
    const sp<String> content = stringtable->getString(filepath, false);
    _includes.push_back(content ? content : sp<String>::make(source));
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

ShaderPreprocessor::DeclarationList::DeclarationList(Source& source, const String& descriptor)
    : _source(source), _descriptor(descriptor)
{
}

void ShaderPreprocessor::DeclarationList::declare(const String& type, const String& prefix, const String& name)
{
    if(!_vars.has(name))
    {
        const sp<String> declared = sp<String>::make(Strings::sprintf("%s %s %s%s;\n", _descriptor.c_str(), type.c_str(), prefix.c_str(), name.c_str()));
        _source.push_back(declared);

        _vars.push_back(name, Declaration(name, type, declared));
    }
    else
        DCHECK(_vars.at(name).type() == type, "Declared type \"\" and variable type \"\" mismatch", _vars.at(name).type().c_str(), type.c_str());
}

void ShaderPreprocessor::DeclarationList::parse(const std::regex& pattern)
{
    _source.replace(pattern, [this](const std::smatch& m) {
        const sp<String> declaration = sp<String>::make(m.str());
        _vars.push_back(m[2].str(), Declaration(m[2].str(), m[1].str(), declaration));
        return declaration;
    });
}

bool ShaderPreprocessor::DeclarationList::has(const String& name) const
{
    return _vars.has(name);
}

const Table<String, ShaderPreprocessor::Declaration>& ShaderPreprocessor::DeclarationList::vars() const
{
    return _vars;
}

Table<String, ShaderPreprocessor::Declaration>& ShaderPreprocessor::DeclarationList::vars()
{
    return _vars;
}

ShaderPreprocessor::Preprocessor::Preprocessor()
    : _type(SHADER_TYPE_NONE)
{
}

ShaderPreprocessor::Preprocessor::Preprocessor(ShaderPreprocessor::ShaderType type, String source)
    : _type(type), _source(std::move(source))
{
}

String ShaderPreprocessor::Preprocessor::process(const RenderContext& glContext) const
{
    DCHECK(glContext.version() > 0, "Unintialized GLContext");

    static std::regex var_pattern("\\$\\{([\\w.]+)\\}");
    const std::map<String, String>& annotations = glContext.annotations();

    return _source.replace(var_pattern, [&annotations] (Array<String>& matches)->String {
        const String& varName = matches.buf()[1];
        const auto iter = annotations.find(varName);
        DCHECK(iter != annotations.end(), "Cannot find constant \"%s\" in RenderEngine", varName.c_str());
        return iter->second;
    });
}

ShaderPreprocessor::Source::Source(String code)
    : _fragments{sp<String>::make(std::move(code))}
{
}

String ShaderPreprocessor::Source::str(char endl) const
{
    StringBuffer sb;
    for(const auto& i : _fragments)
        if(i && !i->empty())
        {
            sb << *i;
            if(endl)
                sb << endl;
        }
    return sb.str();
}

void ShaderPreprocessor::Source::push_front(const sp<String>& fragment)
{
    _fragments.push_front(fragment);
}

void ShaderPreprocessor::Source::push_back(const sp<String>& fragment)
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

void ShaderPreprocessor::Source::replace(const String& str, const String& replacment)
{
    for(const sp<String>& i : _fragments)
        *i = i->replace(str, replacment);
}

void ShaderPreprocessor::Source::replace(const std::regex& regexp, const std::function<sp<String>(const std::smatch&)>& replacer)
{
    for(auto iter = _fragments.begin(); iter != _fragments.end(); ++iter)
    {
        const sp<String>& fragment = *iter;
        std::vector<sp<String>> inserting;
        fragment->search(regexp, [&inserting, replacer](const std::smatch& match) {
            sp<String> replacement = replacer(match);
            if(replacement)
                inserting.push_back(std::move(replacement));
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

const sp<String>& ShaderPreprocessor::Declaration::source() const
{
    return _source;
}

}
