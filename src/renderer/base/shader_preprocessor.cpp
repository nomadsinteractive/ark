#include "renderer/base/shader_preprocessor.h"

#include <regex>

#include "core/types/global.h"

#include "core/base/string_buffer.h"
#include "core/base/string_table.h"
#include "core/types/global.h"
#include "core/util/strings.h"

#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/util/render_util.h"

#define ARRAY_PATTERN           "(?:\\[\\s*(\\d+)\\s*\\])?"
#define STD_TYPE_PATTERN        "int|uint8|float|[bi]?vec[234]|mat3|mat4"
#define ATTRIBUTE_PATTERN       "\\s+(" STD_TYPE_PATTERN ")\\s+" "(?:a_|v_)(\\w+)" ARRAY_PATTERN ";"
#define UNIFORM_PATTERN         "\\s+(\\w+)\\s+" "(u_\\w+)" ARRAY_PATTERN ";"
#define LAYOUT_PATTERN          "layout\\((?:std140|binding\\s*=\\s*(\\d+)|r\\d+[ui]*|[\\s,])+\\)\\s+"

#define INDENT_STR "    "


namespace ark {

const char* ShaderPreprocessor::ANNOTATION_VERT_IN = "${vert.in}";
const char* ShaderPreprocessor::ANNOTATION_VERT_OUT = "${vert.out}";
const char* ShaderPreprocessor::ANNOTATION_FRAG_IN = "${frag.in}";
const char* ShaderPreprocessor::ANNOTATION_FRAG_OUT = "${frag.out}";
const char* ShaderPreprocessor::ANNOTATION_FRAG_COLOR = "${frag.color}";

static std::regex _INCLUDE_PATTERN("#include\\s*[<\"]([^>\"]+)[>\"]");
static std::regex _STRUCT_PATTERN("struct\\s+(\\w+)\\s*\\{([^}]+)\\}\\s*;");
static std::regex _IN_PATTERN("(?:attribute|varying|in)" ATTRIBUTE_PATTERN);
static std::regex _UNIFORM_PATTERN("(?:" LAYOUT_PATTERN ")?uniform" UNIFORM_PATTERN);
static std::regex _SSBO_PATTERN(LAYOUT_PATTERN "(?:(?:read|write)only\\s+)?buffer\\s+(\\w+)");

#ifndef ANDROID
static char _STAGE_ATTR_PREFIX[PipelineInput::SHADER_STAGE_COUNT + 1][4] = {"a_", "v_", "t_", "e_", "g_", "f_", "c_"};
#else
static char _STAGE_ATTR_PREFIX[PipelineInput::SHADER_STAGE_COUNT + 1][4] = {"a_", "v_", "f_", "c_"};
#endif


ShaderPreprocessor::ShaderPreprocessor(sp<String> source, PipelineInput::ShaderStage shaderStage, PipelineInput::ShaderStage preShaderStage)
    : _source(std::move(source)), _shader_stage(shaderStage), _pre_shader_stage(preShaderStage), _version(0), _declaration_ins(_attribute_declarations, shaderStage == PipelineInput::SHADER_STAGE_VERTEX ? ANNOTATION_VERT_IN : ANNOTATION_FRAG_IN),
      _declaration_outs(_attribute_declarations, shaderStage == PipelineInput::SHADER_STAGE_VERTEX ? ANNOTATION_VERT_OUT : ANNOTATION_FRAG_OUT),
      _declaration_uniforms(_uniform_declarations, "uniform"), _declaration_samplers(_uniform_declarations, "uniform"), _pre_main(sp<String>::make()),
      _post_main(sp<String>::make())
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

void ShaderPreprocessor::addOutputVarModifier(String modifier)
{
    _output_var_modifiers.push_back(std::move(modifier));
}

void ShaderPreprocessor::initialize(PipelineBuildingContext& context)
{
    parseMainBlock(_source, context);
    parseDeclarations();
}

void ShaderPreprocessor::initializeAsFirst(PipelineBuildingContext& context)
{
    initialize(context);
    for(const auto& i : _main_block->_args)
        if(i._modifier & ShaderPreprocessor::Parameter::PARAMETER_MODIFIER_IN)
            context.addInputAttribute(Strings::capitalizeFirst(i._name), i._type);
}

static bool sanitizer(const std::smatch& match) {
    DCHECK_WARN(false, match.str().c_str());
    return false;
}

void ShaderPreprocessor::parseMainBlock(const String& source, PipelineBuildingContext& buildingContext)
{
    if(source.find("void main()") != String::npos)
    {
        DCHECK_WARN(false, "Shader which contains main function will not be preprocessed by ark shader preprocessor. Try to replace it with \"vec4 ark_main(vec4 position, ...)\" for better flexibilty and compatibilty");
        return;
    }

    DCHECK_WARN(source.search(_IN_PATTERN, sanitizer), "Non-standard attribute declared above, move it into ark_main function's parameters will disable this warning.");

    static const std::regex FUNC_PATTERN("(vec4|void)\\s+ark_main\\(([^)]*)\\)");

    source.search(FUNC_PATTERN, [this] (const std::smatch& m)->bool {
        const String prefix = m.prefix().str();
        const String remaining = m.suffix().str();
        String body;
        size_t prefixStart = parseFunctionBody(remaining, body);
        sp<String> fragment = sp<String>::make();
        _main.push_back(sp<String>::make(prefix));
        _main.push_back(fragment);
        _main.push_back(sp<String>::make(remaining.substr(prefixStart)));
        _main_block = sp<Function>::make("main", m[2].str(), m[1].str(), body.strip(), std::move(fragment));
        return false;
    });

    DCHECK(_main_block, "Parsing source error: \n%s\n Undefined ark_main in shader", source.c_str());

    _main_block->parse(buildingContext);
}

void ShaderPreprocessor::parseDeclarations()
{
    _main.replace(_INCLUDE_PATTERN, [this](const std::smatch& m) {
        this->addInclude(m.str(), m[1].str());
        return nullptr;
    });

    auto structPatternReplacer = [this](const std::smatch& m) {
        const sp<String> declaration = sp<String>::make(m.str());
        this->_struct_declarations.push_back(declaration);
        this->_struct_definitions.push_back(m[1].str(), m[2].str());
        return nullptr;
    };
    _includes.replace(_STRUCT_PATTERN, structPatternReplacer);
    _main.replace(_STRUCT_PATTERN, structPatternReplacer);

    _main.replace(_UNIFORM_PATTERN, [this](const std::smatch& m) {
        const sp<String> declaration = sp<String>::make(m.str());
        uint32_t length = m[4].str().empty() ? 1 : Strings::parse<uint32_t>(m[4].str());
        this->addUniform(m[2].str(), m[3].str(), length, declaration);
        return nullptr;
    });

    auto ssboPattern = [this](const std::smatch& m) {
        _ssbos[m[2].str()] = Strings::parse<int32_t>(m[1].str());
        return true;
    };
    _includes.search(_SSBO_PATTERN, ssboPattern);
    _main.search(_SSBO_PATTERN, ssboPattern);

    if(!_main_block)
        return;

    _main_block->genDefinition();

    {
        const String outVar = outputName();
        _main.push_back(sp<String>::make("\n\nvoid main() {\n"));
        _main.push_back(_pre_main);
        if(outVar && _main_block->hasReturnValue())
            _main.push_back(sp<String>::make(Strings::sprintf(INDENT_STR "%s = ", outVar.c_str())));
        _main.push_back(sp<String>::make(_main_block->genOutCall(_pre_shader_stage, _shader_stage)));
        for(const String& i : _output_var_modifiers)
        {
            _main.push_back(sp<String>::make(" * "));
            _main.push_back(sp<String>::make(i));
        }
        _main.push_back(sp<String>::make(";"));
        _main.push_back(_post_main);
        _main.push_back(sp<String>::make("\n}\n\n"));
    }
}

ShaderPreprocessor::Preprocessed ShaderPreprocessor::preprocess()
{
    return Preprocessed(_shader_stage, genDeclarations(_main.str()));
}

void ShaderPreprocessor::setupUniforms(Table<String, sp<Uniform>>& uniforms, int32_t& binding)
{
    for(const auto& i : _declaration_uniforms.vars())
    {
        const String& name = i.first;
        if(!uniforms.has(name))
        {
            const Declaration& declare = i.second;
            Uniform::Type type = Uniform::toType(declare.type());
            uniforms.push_back(name, sp<Uniform>::make(name, declare.type(), type, type == Uniform::TYPE_STRUCT ? getUniformSize(type, declare.type())
                                                                                                                : Uniform::getComponentSize(type), declare.length(), nullptr));
        }
    }

    int32_t next = binding;
    for(const sp<Uniform>& i : uniforms.values())
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

            if(!_declaration_uniforms.has(i->name()))
            {
                const String type = i->declaredType();
                sp<String> declaration = sp<String>::make(i->declaration("uniform "));
                _declaration_uniforms.vars().push_back(i->name(), Declaration(i->name(), type, i->length(), declaration));
                if(pos == String::npos)
                    _uniform_declarations.push_back(std::move(declaration));
            }
        }
    }
    binding = next;
}

const char* ShaderPreprocessor::inVarPrefix() const
{
    return _STAGE_ATTR_PREFIX[_pre_shader_stage + 1];
}

const char* ShaderPreprocessor::outVarPrefix() const
{
    return _STAGE_ATTR_PREFIX[_shader_stage + 1];
}

const std::vector<ShaderPreprocessor::Parameter>& ShaderPreprocessor::args() const
{
    return _main_block->_args;
}

void ShaderPreprocessor::inDeclare(const String& type, const String& name, int32_t location)
{
    _declaration_ins.declare(type, inVarPrefix(), name, location, nullptr, _shader_stage == PipelineInput::SHADER_STAGE_FRAGMENT && type == "int");
}

void ShaderPreprocessor::outDeclare(const String& type, const String& name, int32_t location)
{
    _declaration_outs.declare(type, outVarPrefix(), name, location, nullptr, type == "int");
}

void ShaderPreprocessor::passThroughDeclare(const String& type, const String& name, int32_t location)
{
    outDeclare(type, name, location);
    addPreMainSource(Strings::sprintf("%s%s = %s%s;", outVarPrefix(), name.c_str(), inVarPrefix(), name.c_str()));
}

void ShaderPreprocessor::linkNextStage(const String& returnValueName)
{
    const char* varPrefix = outVarPrefix();
    int32_t location = -1;
    if(_main_block->hasReturnValue())
        _declaration_outs.declare(_main_block->_return_type, varPrefix, returnValueName, ++location);
    for(const Parameter& i : _main_block->_args)
        if(i._modifier == ShaderPreprocessor::Parameter::PARAMETER_MODIFIER_OUT)
            _declaration_outs.declare(i._type, varPrefix, Strings::capitalizeFirst(i._name), ++location, i.getQualifierStr());
}

void ShaderPreprocessor::linkPreStage(const ShaderPreprocessor& preStage, std::set<String>& passThroughVars)
{
    linkParameters(_predefined_parameters, preStage, passThroughVars);
    linkParameters(_main_block->_args, preStage, passThroughVars);
}

sp<Uniform> ShaderPreprocessor::getUniformInput(const String& name, Uniform::Type type) const
{
    if(!_declaration_uniforms.has(name))
        return nullptr;

    const Declaration& declaration = _declaration_uniforms.vars().at(name);
    DCHECK(Uniform::toType(declaration.type()) == type, "Uniform \"%s\" declared type: %s, but it should be %d", name.c_str(), declaration.type().c_str(), type);
    return sp<Uniform>::make(name, type, 1, nullptr);
}

String ShaderPreprocessor::outputName() const
{
#ifndef ANDROID
    static const char* sOutputNames[PipelineInput::SHADER_STAGE_COUNT] = {"gl_Position", "", "", "", ANNOTATION_FRAG_COLOR, ""};
#else
    static const char* sOutputNames[PipelineInput::SHADER_STAGE_COUNT] = {"gl_Position", ANNOTATION_FRAG_COLOR, ""};
#endif
    return sOutputNames[_shader_stage];
}

size_t ShaderPreprocessor::parseFunctionBody(const String& s, String& body) const
{
    String::size_type pos = s.find('{');
    DCHECK(pos != String::npos, "Cannot parse function body: %s", s.c_str());
    size_t end = Strings::parentheses(s, pos, '{', '}');
    body = s.substr(pos + 1, end - 1).strip();
    return end + 1;
}

void ShaderPreprocessor::addUniform(const String& type, const String& name, uint32_t length, const sp<String>& declaration)
{
    Declaration uniform(name, type, length, declaration);
    if(type.startsWith("sampler"))
        _declaration_samplers.vars().push_back(name, std::move(uniform));
    else
        _declaration_uniforms.vars().push_back(name, std::move(uniform));
    _uniform_declarations.push_back(declaration);
}

uint32_t ShaderPreprocessor::getUniformSize(Uniform::Type type, const String& declaredType) const
{
    if(type != Uniform::TYPE_STRUCT)
        return Uniform::getComponentSize(type);

    const String source = _struct_definitions.at(declaredType);
    uint32_t size = 0;
    for(const String& i : source.split(';'))
    {
        String vtype, vname;
        Strings::cut(i.strip(), vtype, vname, ' ');
        Uniform::Type t = Uniform::toType(vtype);
        size += getUniformSize(t, vtype.strip());
    }
    return size;
}

void ShaderPreprocessor::linkParameters(const std::vector<ShaderPreprocessor::Parameter>& parameters, const ShaderPreprocessor& preStage, std::set<String>& passThroughVars)
{
    for(const auto& i : parameters)
        if(i._modifier & Parameter::PARAMETER_MODIFIER_IN)
            if(!preStage._main_block->hasOutAttribute(i._name))
                passThroughVars.insert(Strings::capitalizeFirst(i._name));
}

const char* ShaderPreprocessor::getOutAttributePrefix(PipelineInput::ShaderStage preStage)
{
    return _STAGE_ATTR_PREFIX[preStage + 1];
}

String ShaderPreprocessor::genDeclarations(const String& mainFunc) const
{
    StringBuffer sb;
    if(_version && !_main.contains("#version "))
        sb << "#version " << _version << '\n';

    sb << '\n';
    if(_predefined_macros.size())
    {
        for(const String& i : _predefined_macros)
            sb << i << '\n';
        sb << '\n';
    }

    sb << _struct_declarations.str('\n');
    sb << _includes.str('\n');
    sb << _uniform_declarations.str('\n');
    sb << _attribute_declarations.str('\n');
    sb << mainFunc;
    return sb.str();
}

void ShaderPreprocessor::addInclude(const String& source, const String& filepath)
{
    const Global<StringTable> stringtable;
    sp<String> content;
    const String::size_type pos = filepath.find(':');
    if(pos == String::npos)
        content = stringtable->getString(filepath, false);
    else
        content = stringtable->getString(filepath.substr(0, pos), filepath.substr(pos + 1).lstrip('/'), false);
    CHECK(content, "Can't open include file \"%s\"", filepath.c_str());
    _includes.push_back(content ? std::move(content) : sp<String>::make(source));
}

ShaderPreprocessor::Function::Function(String name, String params, String returnType, String body, sp<String> placeHolder)
    : _name(std::move(name)), _params(std::move(params)), _return_type(std::move(returnType)), _body(std::move(body)), _place_hoder(std::move(placeHolder))
{
}

void ShaderPreprocessor::Function::parse(PipelineBuildingContext& buildingContext)
{
    uint32_t stride = 0;
    for(const String& i : _params.split(','))
    {
        const String s = i.strip();
        Parameter param = parseParameter(s);
        if(param._modifier & Parameter::PARAMETER_MODIFIER_OUT)
        {
            const Attribute attr = RenderUtil::makePredefinedAttribute(param._name, param._type);
            CHECK_WARN(attr.length() != 3 || stride % 16 == 0, "3-component out attribute \"%s\" ranged from %d to %d, some GPUs may not like this", attr.name().c_str(), stride, stride + attr.size());
            stride += attr.size();
        }
        if(param._modifier & Parameter::PARAMETER_MODIFIER_IN)
            buildingContext.addPredefinedAttribute(Strings::capitalizeFirst(param._name), param._type, PipelineInput::SHADER_STAGE_VERTEX);

        _args.push_back(std::move(param));
    }
}

ShaderPreprocessor::Parameter ShaderPreprocessor::Function::parseParameter(const String& param)
{
    int32_t modifier = Parameter::PARAMETER_MODIFIER_DEFAULT;
    String type, name;
    for(const String& i : param.split(' '))
    {
        if(i == "in")
        {
            DCHECK(modifier == Parameter::PARAMETER_MODIFIER_DEFAULT, "Conflicts found in parameter(%s)'s qualifier", param.c_str());
            modifier = Parameter::PARAMETER_MODIFIER_IN;
            continue;
        }
        if(i == "out")
        {
            DCHECK(modifier == Parameter::PARAMETER_MODIFIER_DEFAULT, "Conflicts found in parameter(%s)'s qualifier", param.c_str());
            modifier = Parameter::PARAMETER_MODIFIER_OUT;
            continue;
        }
        if(i == "inout")
        {
            DCHECK(modifier == Parameter::PARAMETER_MODIFIER_DEFAULT, "Conflicts found in parameter(%s)'s qualifier", param.c_str());
            modifier = Parameter::PARAMETER_MODIFIER_INOUT;
            continue;
        }
        if(!type)
        {
            type = i;
            continue;
        }
        DASSERT(!name);
        name = i;
    }
    DCHECK(type && name, "Cannot parse function arguments: %s", param.c_str());
    return Parameter(std::move(type), std::move(name), static_cast<Parameter::Modifier>(modifier == Parameter::PARAMETER_MODIFIER_DEFAULT ? Parameter::PARAMETER_MODIFIER_IN : modifier));
}

void ShaderPreprocessor::Function::genDefinition()
{
    StringBuffer sb;
    sb << _return_type << " ark_" << _name << "(";

    const auto begin = _args.begin();
    for(auto iter = begin; iter != _args.end(); ++iter)
    {
        if(iter != begin)
            sb << ", ";
        sb << iter->getQualifierStr() << " " << iter->_type << " " << iter->_name;
    }

    sb << ") {\n    " << _body << "\n}";
    *_place_hoder = sb.str();
}

String ShaderPreprocessor::Function::genOutCall(PipelineInput::ShaderStage preShaderStage, PipelineInput::ShaderStage shaderStage) const
{
    StringBuffer sb;
    sb << "ark_main(";
    const auto begin = _args.begin();
    for(auto iter = begin; iter != _args.end(); ++iter)
    {
        if(iter != begin)
            sb << ", ";
        if(iter->_modifier & Parameter::PARAMETER_MODIFIER_OUT)
            sb << getOutAttributePrefix(shaderStage);
        else
            sb << getOutAttributePrefix(preShaderStage);
        sb << Strings::capitalizeFirst(iter->_name);
    }

    sb << ')';
    return sb.str();
}

bool ShaderPreprocessor::Function::hasOutAttribute(const String& name) const
{
    for(const auto& i : _args)
        if(i._modifier & Parameter::PARAMETER_MODIFIER_OUT && i._name == name)
            return true;
    return false;
}

bool ShaderPreprocessor::Function::hasReturnValue() const
{
    return _return_type != "void";
}

size_t ShaderPreprocessor::Function::outArgumentCount() const
{
    size_t count = 0;
    for(const auto& i : _args)
        if(i._modifier & Parameter::PARAMETER_MODIFIER_OUT)
            count ++;
    return count;
}

ShaderPreprocessor::DeclarationList::DeclarationList(Source& source, const String& descriptor)
    : _source(source), _descriptor(descriptor)
{
}

void ShaderPreprocessor::DeclarationList::declare(const String& type, const char* prefix, const String& name, int32_t location, const char* qualifier, bool isFlat)
{
    if(!_vars.has(name))
    {
        sp<String> declared = sp<String>::make(Strings::sprintf("%s%s %s %s%s;", isFlat ? "flat " : "", qualifier ? qualifier : _descriptor.c_str(), type.c_str(), prefix, name.c_str()));
        if(location >= 0)
            _source.push_back(sp<String>::make(Strings::sprintf("layout (location = %d) %s", location, declared->c_str())));
        else
            _source.push_back(declared);

        _vars.push_back(name, Declaration(name, type, 1, declared));
    }
    else
        CHECK(_vars.at(name).type() == type, "Declared type \"\" and variable type \"\" mismatch", _vars.at(name).type().c_str(), type.c_str());
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

ShaderPreprocessor::Preprocessed::Preprocessed()
    : _type(PipelineInput::SHADER_STAGE_NONE)
{
}

ShaderPreprocessor::Preprocessed::Preprocessed(PipelineInput::ShaderStage stage, String source)
    : _type(stage), _source(std::move(source))
{
}

PipelineInput::ShaderStage ShaderPreprocessor::Preprocessed::stage() const
{
    return _type;
}

String ShaderPreprocessor::Preprocessed::toSourceCode(const RenderEngineContext& renderEngineContext, const std::map<String, String>& definitions) const
{
    DCHECK(renderEngineContext.version() > 0, "Unintialized RenderEngineContext");

    static std::regex var_pattern("\\$\\{([\\w.]+)\\}");
    const std::map<String, String>& engineDefinitions = renderEngineContext.definitions();

    return _source.replace(var_pattern, [&engineDefinitions, &definitions] (Array<String>& matches)->String {
        const String& varName = matches.at(1);
        auto iter = engineDefinitions.find(varName);
        if(iter != engineDefinitions.end())
            return iter->second;
        iter = definitions.find(varName);
        CHECK(iter != definitions.end(), "Undefinition \"%s\"", varName.c_str());
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

ShaderPreprocessor::Declaration::Declaration(const String& name, const String& type, uint32_t length, const sp<String>& source)
    : _name(name), _type(type), _length(length), _source(source)
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

uint32_t ShaderPreprocessor::Declaration::length() const
{
    return _length;
}

const sp<String>& ShaderPreprocessor::Declaration::source() const
{
    return _source;
}

ShaderPreprocessor::Parameter::Parameter()
    : _modifier(PARAMETER_MODIFIER_DEFAULT)
{
}

ShaderPreprocessor::Parameter::Parameter(String type, String name, ShaderPreprocessor::Parameter::Modifier modifier)
    : _type(std::move(type)), _name(std::move(name)), _modifier(modifier)
{
}

const char* ShaderPreprocessor::Parameter::getQualifierStr() const
{
    const char* qualifiers[] = {"in", "in", "out", "inout"};
    DASSERT(_modifier >= PARAMETER_MODIFIER_DEFAULT && _modifier <= PARAMETER_MODIFIER_INOUT);
    return qualifiers[_modifier];
}

}
