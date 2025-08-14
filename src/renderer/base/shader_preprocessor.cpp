#include "renderer/base/shader_preprocessor.h"

#include <regex>

#include "core/types/global.h"

#include "core/base/string_buffer.h"
#include "core/base/string_table.h"
#include "core/util/strings.h"

#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/util/render_util.h"

#define ARRAY_PATTERN           "(?:\\[\\s*(\\d+)\\s*\\])?"
#define STD_TYPE_PATTERN        "int|uint8|float|[bi]?vec[234]|mat3|mat4"
#define ATTRIBUTE_PATTERN       "\\s+(" STD_TYPE_PATTERN ")\\s+" "(?:a_|v_)(\\w+)" ARRAY_PATTERN ";"
#define UNIFORM_PATTERN         "(\\w+)\\s+" "(\\w+)" ARRAY_PATTERN ";"
#define LAYOUT_PATTERN          R"--(layout\((?:packed|shared|std140|std430|binding\s*=\s*(\d+)|set\s*=\s*(\d+)|[rgba]+\d+[uif]*|location=\d+|[\s,])+\)\s+)--"
#define ACCESSIBILITY_PATTERN   "(?:(?:read|write)only\\s+)?"

#define INDENT_STR "    "

namespace ark {

namespace {

const std::regex REGEX_INCLUDE_PATTERN(R"(#include\s*[<"]([^>"]+)[>"])");
const std::regex REGEX_STRUCT_PATTERN(R"(struct\s+(\w+)\s*\{([^}]+)\}\s*;)");
const std::regex REGEX_IN_PATTERN("(?:attribute|varying|in)" ATTRIBUTE_PATTERN);
const std::regex REGEX_UNIFORM_PATTERN("(?:" LAYOUT_PATTERN ")?uniform\\s+" ACCESSIBILITY_PATTERN UNIFORM_PATTERN);
const std::regex REGEX_SSBO_PATTERN(LAYOUT_PATTERN ACCESSIBILITY_PATTERN "buffer\\s+(\\w+)");
const std::regex REGEX_LOCAL_SIZE_PATTERN(R"--(layout\s*\(((?:local_size_[xyz]\s*=\s*\d+\s*,?\s*)+)\)\s+in\s*;)--");

#ifndef ANDROID
constexpr char STAGE_ATTR_PREFIX[enums::SHADER_STAGE_BIT_COUNT + 1][4] = {"a_", "v_", "t_", "e_", "g_", "f_", "c_"};
#else
char STAGE_ATTR_PREFIX[Enum::SHADER_STAGE_COUNT + 1][4] = {"a_", "v_", "f_", "c_"};
#endif

constexpr char ANNOTATION_VERT_IN[] = "in";
constexpr char ANNOTATION_VERT_OUT[] = "out";
constexpr char ANNOTATION_FRAG_IN[] = "in";
constexpr char ANNOTATION_FRAG_OUT[] = "out";
constexpr char ANNOTATION_FRAG_COLOR[] = "f_FragColor";

size_t parseFunctionBody(const String& s, String& body)
{
    const size_t end = Strings::parentheses(s, 0, '{', '}', 1);
    body = s.substr(0, end - 1).strip();
    return end + 1;
}

bool sanitizer(const std::smatch& match)
{
    WARN(match.str().c_str());
    return false;
}

}

ShaderPreprocessor::ShaderPreprocessor(String source, document manifest, const enums::ShaderStageBit shaderStage, const enums::ShaderStageBit preShaderStage)
    : _source(std::move(source)), _manifest(std::move(manifest)), _shader_stage(shaderStage), _pre_shader_stage(preShaderStage), _version(0), _declaration_ins(_attribute_declaration_codes, shaderStage == enums::SHADER_STAGE_BIT_VERTEX ? ANNOTATION_VERT_IN : ANNOTATION_FRAG_IN),
      _declaration_outs(_attribute_declaration_codes, shaderStage == enums::SHADER_STAGE_BIT_VERTEX ? ANNOTATION_VERT_OUT : ANNOTATION_FRAG_OUT),
      _declaration_uniforms(_uniform_declaration_codes, "uniform"), _declaration_samplers(_uniform_declaration_codes, "uniform"), _declaration_images(_uniform_declaration_codes, "uniform"),
      _pre_main(sp<String>::make()), _post_main(sp<String>::make())
{
    _predefined_macros.push_back(Strings::sprintf("#define ARK_Z_DIRECTION %.2f", Ark::instance().renderController()->renderEngine()->toLayoutDirection(1.0f)));
}

void ShaderPreprocessor::addPreMainSource(const String& source)
{
    *_pre_main = Strings::sprintf(INDENT_STR "%s\n%s", source.c_str(), _pre_main->c_str());
}

void ShaderPreprocessor::addPostMainSource(const String& source)
{
    *_post_main = Strings::sprintf("%s\n" INDENT_STR "%s", _post_main->c_str(), source.c_str());
}

void ShaderPreprocessor::addOutputModifier(String preModifier, String postModifier)
{
    _result_modifiers.push_back({std::move(preModifier), std::move(postModifier)});
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
        if(i._annotation & Parameter::PARAMETER_ANNOTATION_IN)
            context.addAttribute(Strings::capitalizeFirst(i._name), i._type, i._divisor);
}

void ShaderPreprocessor::parseMainBlock(const String& source, PipelineBuildingContext& buildingContext)
{
    if(source.find("void main()") != String::npos)
    {
        CHECK_WARN(false, "Shader which contains main function will not be preprocessed by ark shader preprocessor. Try to replace it with \"vec4 ark_main(vec4 position, ...)\" for better flexibilty and compatibilty");
        return;
    }

    CHECK_WARN(source.search(REGEX_IN_PATTERN, sanitizer), "Non-standard attribute declared above, move it into ark_main function's parameters will disable this warning.");

    static const std::regex FUNC_PATTERN(R"((vec4|void)\s+ark_main\(([^{]*)\)[\s\r\n]*\{)");

    source.search(FUNC_PATTERN, [this] (const std::smatch& m)->bool {
        const String prefix = m.prefix().str();
        const String remaining = m.suffix().str();
        String body;
        const size_t prefixStart = parseFunctionBody(remaining, body);
        sp<String> fragment = sp<String>::make();
        _main.push_back(sp<String>::make(prefix + "\n"));
        _main.push_back(fragment);
        _main.push_back(sp<String>::make("\n" + remaining.substr(prefixStart)));
        _main_block = sp<Function>::make("main", m[2].str(), m[1].str(), body.strip(), std::move(fragment));
        return false;
    });

    DCHECK(_main_block, "Parsing source error: \n%s\n Undefined ark_main in shader", source.c_str());

    _main_block->parse(buildingContext);
}

void ShaderPreprocessor::parseDeclarations()
{
    this->addInclude("shaders/defines.h");
    _main.replace(REGEX_INCLUDE_PATTERN, [this](const std::smatch& m) {
        this->addInclude(m[1].str());
        return nullptr;
    });

    const auto structPatternReplacer = [this](const std::smatch& m) {
        const sp<String> declaration = sp<String>::make(m.str());
        this->_struct_declaration_codes.push_back(declaration);
        this->_struct_definitions.push_back(m[1].str(), m[2].str());
        return nullptr;
    };
    _include_declaration_codes.replace(REGEX_STRUCT_PATTERN, structPatternReplacer);
    _main.replace(REGEX_STRUCT_PATTERN, structPatternReplacer);

    const auto uniformPatternReplacer = [this](const std::smatch& m) {
        const uint32_t length = m[5].matched ? Strings::eval<uint32_t>(m[5].str()) : 1;
        return this->addUniform(m[3].str(), m[4].str(), length, m.str());
    };
    _include_declaration_codes.replace(REGEX_UNIFORM_PATTERN, uniformPatternReplacer);
    _main.replace(REGEX_UNIFORM_PATTERN, uniformPatternReplacer);

    auto ssboPattern = [this](const std::smatch& m) {
        const int32_t binding = m[1].matched ? Strings::eval<int32_t>(m[1].str()) : -1;
        const int32_t set = m[2].matched ? Strings::eval<int32_t>(m[2].str()) : -1;
        _ssbos[m[3].str()] = {binding, set};
        return true;
    };
    _include_declaration_codes.search(REGEX_SSBO_PATTERN, ssboPattern);
    _main.search(REGEX_SSBO_PATTERN, ssboPattern);

    if(_shader_stage == enums::SHADER_STAGE_BIT_COMPUTE)
    {
        const auto localSizeTraveller = [this] (const std::smatch& m)
        {
            _compute_local_sizes = {{1, 1, 1}};
            V3i& localSizes = _compute_local_sizes.value();
            for(const String& i : String(m[1].str()).split(','))
                if(const auto [k, v] = i.cut('='); v)
                {
                    const uint32_t workGroup = k.strip().at(StringView("local_size_").size()) - 'x';
                    const int32_t size = Strings::eval<int32_t>(v.value());
                    localSizes[workGroup] = size;
                }
            return true;
        };
        _main.search(REGEX_LOCAL_SIZE_PATTERN, localSizeTraveller);
    }

    if(!_main_block)
        return;

    _main_block->genDefinition();

    {
        const String outVar = outputName();
        _main.push_back(sp<String>::make("\n\nvoid main() {\n"));
        _main.push_back(_pre_main);
        if(outVar && _main_block->hasReturnValue())
            _main.push_back(sp<String>::make(Strings::sprintf(INDENT_STR "%s = ", outVar.c_str())));
        const sp<String> preModifier = sp<String>::make();
        _main.push_back(preModifier);
        _main.push_back(sp<String>::make(_main_block->genOutCall(_pre_shader_stage, _shader_stage)));
        for(const auto& [pre, post] : _result_modifiers)
        {
            if(pre)
                *preModifier = *preModifier + pre;
            _main.push_back(sp<String>::make(post));
        }
        _main.push_back(sp<String>::make(";"));
        _main.push_back(_post_main);
        _main.push_back(sp<String>::make("\n}\n\n"));
    }
}

ShaderPreprocessor::Stage ShaderPreprocessor::preprocess() const
{
    return {_manifest, _shader_stage, genDeclarations(_main.str())};
}

void ShaderPreprocessor::setupUniforms(Table<String, sp<Uniform>>& uniforms)
{
    for(const auto& [name, declare] : _declaration_uniforms.vars())
        if(!uniforms.has(name))
        {
            Uniform::Type type = Uniform::toType(declare.type());
            uniforms.push_back(name, sp<Uniform>::make(name, declare.type(), type, type == Uniform::TYPE_STRUCT ? getUniformSize(type, declare.type())
                                                                                                                : Uniform::getComponentSize(type), declare.length(), nullptr));
        }

    for(const sp<Uniform>& i : uniforms.values())
    {
        const String::size_type pos = i->name().find('[');
        DCHECK(pos != 0, "Illegal uniform name: %s", i->name().c_str());
        if(_main.contains(pos == String::npos ? i->name() : i->name().substr(0, pos)) && !_declaration_uniforms.has(i->name()))
        {
            const String type = i->declaredType();
            sp<String> declaration = sp<String>::make(i->declaration("uniform "));
            _declaration_uniforms.vars().push_back(i->name(), Declaration(i->name(), type, i->length(), declaration));
            if(pos == String::npos)
                _uniform_declaration_codes.push_back(std::move(declaration));
        }
    }
}

const char* ShaderPreprocessor::inVarPrefix() const
{
    return STAGE_ATTR_PREFIX[_pre_shader_stage + 1];
}

const char* ShaderPreprocessor::outVarPrefix() const
{
    return STAGE_ATTR_PREFIX[_shader_stage + 1];
}

const Vector<ShaderPreprocessor::Parameter>& ShaderPreprocessor::args() const
{
    return _main_block->_args;
}

void ShaderPreprocessor::inDeclare(const String& type, const String& name)
{
    _declaration_ins.declare(type, inVarPrefix(), name, "", nullptr, _shader_stage == enums::SHADER_STAGE_BIT_FRAGMENT && (type == "int" || type == "uint" || type.startsWith("ivec") || type.startsWith("uvec")));
}

void ShaderPreprocessor::outDeclare(const String& type, const String& name)
{
    _declaration_outs.declare(type, outVarPrefix(), name, "", nullptr, type == "int");
}

void ShaderPreprocessor::passThroughDeclare(const String& type, const String& name)
{
    outDeclare(type, name);
    addPreMainSource(Strings::sprintf("%s%s = %s%s;", outVarPrefix(), name.c_str(), inVarPrefix(), name.c_str()));
}

void ShaderPreprocessor::linkNextStage(const String& returnValueName)
{
    const char* varPrefix = outVarPrefix();
    int32_t location = -1;
    if(_main_block->hasReturnValue())
        _declaration_outs.declare(_main_block->_return_type, varPrefix, returnValueName, Strings::sprintf("location = %d", ++location));
    for(const Parameter& i : _main_block->_args)
        if(i._annotation == Parameter::PARAMETER_ANNOTATION_OUT)
            _declaration_outs.declare(i._type, varPrefix, Strings::capitalizeFirst(i._name), Strings::sprintf("location = %d", ++location), i.getQualifierStr());
}

void ShaderPreprocessor::linkPreStage(const ShaderPreprocessor& preStage, Set<String>& passThroughVars) const
{
    linkParameters(_predefined_parameters, preStage, passThroughVars);
    linkParameters(_main_block->_args, preStage, passThroughVars);
}

sp<Uniform> ShaderPreprocessor::makeUniformInput(String name, Uniform::Type type) const
{
    if(!_declaration_uniforms.has(name))
        return nullptr;

    const Declaration& declaration = _declaration_uniforms.vars().at(name);
    DCHECK(Uniform::toType(declaration.type()) == type, "Uniform \"%s\" declared type: %s, but it should be %d", name.c_str(), declaration.type().c_str(), type);
    return sp<Uniform>::make(std::move(name), type, 1, nullptr);
}

void ShaderPreprocessor::insertUBOStruct(const PipelineLayout::UBO& ubo, const int32_t spaceSet)
{
    StringBuffer sb;
    sb << "layout (";
    if(spaceSet >= 0)
        sb << "set = " << spaceSet << ", ";
    sb << "binding = " << ubo.binding() << ") uniform UBO" << ubo.binding() << " {\n";
    for(const auto& i : ubo.uniforms().values()) {
        _main.replace(i->name(), Strings::sprintf("ubo%d.%s", ubo.binding(), i->name().c_str()));
        sb << i->declaration("") << '\n';
    }
    sb << "} ubo" << ubo.binding() << ";\n\n";
    _uniform_declaration_codes.push_back(sp<String>::make(sb.str()));
}

bool ShaderPreprocessor::hasUBO(const PipelineLayout::UBO& ubo) const
{
    if(ubo._stages.has(_shader_stage))
    {
        for(const auto& i : ubo.uniforms().values())
            if(!_declaration_uniforms.has(i->name()))
                return false;
        return true;
    }
    return false;
}

void ShaderPreprocessor::declareUBOStruct(const PipelineLayout& pipelineLayout, const int32_t spaceSet)
{
    for(const Declaration& i : _declaration_uniforms.vars().values())
        i.setSource("");

    for(const sp<PipelineLayout::UBO>& i : pipelineLayout.ubos())
        if(hasUBO(i))
            insertUBOStruct(i, spaceSet);
}

String ShaderPreprocessor::outputName() const
{
#ifndef ANDROID
    constexpr StringView sOutputNames[enums::SHADER_STAGE_BIT_COUNT] = {"gl_Position", "", "", "", ANNOTATION_FRAG_COLOR, ""};
#else
    constexpr StringView sOutputNames[Enum::SHADER_STAGE_COUNT] = {"gl_Position", ANNOTATION_FRAG_COLOR, ""};
#endif
    return {sOutputNames[_shader_stage].data()};
}

sp<String> ShaderPreprocessor::addUniform(const String& type, const String& name, const uint32_t length, String declaration)
{
    sp<String> declarationVar = sp<String>::make(std::move(declaration));
    Declaration uniform(name, type, length, declarationVar);
    const auto imageTypePos = type.find("image");
    if(type.startsWith("sampler"))
        _declaration_samplers.vars().push_back(name, std::move(uniform));
    else if(imageTypePos == 0 || imageTypePos == 1)
        _declaration_images.vars().push_back(name, std::move(uniform));
    else
    {
        _declaration_uniforms.vars().push_back(name, std::move(uniform));
        _uniform_declaration_codes.push_back(std::move(declarationVar));
        return nullptr;
    }
    return declarationVar;
}

uint32_t ShaderPreprocessor::getUniformSize(Uniform::Type type, const String& declaredType) const
{
    if(type != Uniform::TYPE_STRUCT)
        return Uniform::getComponentSize(type);

    const String source = _struct_definitions.at(declaredType);
    uint32_t size = 0;
    for(const String& i : source.split(';'))
    {
        auto [vtype, vname] = i.strip().cut(' ');
        const Uniform::Type t = Uniform::toType(vtype);
        size += getUniformSize(t, vtype.strip());
    }
    return size;
}

void ShaderPreprocessor::linkParameters(const Vector<Parameter>& parameters, const ShaderPreprocessor& preStage, Set<String>& passThroughVars)
{
    for(const auto& i : parameters)
        if(i._annotation & Parameter::PARAMETER_ANNOTATION_IN)
            if(!preStage._main_block->hasOutAttribute(i._name))
                passThroughVars.insert(Strings::capitalizeFirst(i._name));
}

const char* ShaderPreprocessor::getOutAttributePrefix(enums::ShaderStageBit preStage)
{
    return STAGE_ATTR_PREFIX[preStage + 1];
}

String ShaderPreprocessor::genDeclarations(const String& mainFunc) const
{
    StringBuffer sb;
    if(_version && !_main.contains("#version "))
        sb << "#version " << _version << '\n';

    sb << '\n';
    if(!_predefined_macros.empty())
    {
        for(const String& i : _predefined_macros)
            sb << i << '\n';
        sb << '\n';
    }

    sb << _struct_declaration_codes.str('\n');
    sb << _include_declaration_codes.str('\n');
    sb << _uniform_declaration_codes.str('\n');
    sb << _attribute_declaration_codes.str('\n');
    sb << mainFunc;
    return sb.str();
}

void ShaderPreprocessor::addInclude(const String& filepath)
{
    const Global<StringTable> stringtable;

    Optional<String> content;
    if(const String::size_type pos = filepath.find(':'); pos == String::npos)
        content = stringtable->getString(filepath, false);
    else
        content = stringtable->getString(filepath.substr(0, pos), filepath.substr(pos + 1).lstrip('/'), false);
    CHECK(content, "Can't open include file \"%s\"", filepath.c_str());
    _include_declaration_codes.push_back(content ? sp<String>::make(std::move(content.value())) : sp<String>());
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
        if(param._annotation & Parameter::PARAMETER_ANNOTATION_OUT)
        {
            const Attribute attr = RenderUtil::makePredefinedAttribute(param._name, param._type);
            CHECK_WARN(attr.length() != 3 || stride % 16 == 0, "3-component out attribute \"%s\" ranged from %d to %d, some GPUs may not like this", attr.name().c_str(), stride, stride + attr.size());
            stride += attr.size();
        }
        if(param._annotation & Parameter::PARAMETER_ANNOTATION_IN)
            buildingContext.addPredefinedAttribute(Strings::capitalizeFirst(param._name), param._type, param._divisor, enums::SHADER_STAGE_BIT_VERTEX);

        _args.push_back(std::move(param));
    }
}

ShaderPreprocessor::Parameter ShaderPreprocessor::Function::parseParameter(const String& param)
{
    String type, name;
    uint32_t divisor = 0;
    int32_t modifier = Parameter::PARAMETER_ANNOTATION_DEFAULT;
    for(const String& i : param.split(' '))
    {
        if(i == "in")
        {
            DCHECK(modifier == Parameter::PARAMETER_ANNOTATION_DEFAULT, "Conflicts found in parameter(%s)'s qualifier", param.c_str());
            modifier = Parameter::PARAMETER_ANNOTATION_IN;
            continue;
        }
        if(i == "out")
        {
            DCHECK(modifier == Parameter::PARAMETER_ANNOTATION_DEFAULT, "Conflicts found in parameter(%s)'s qualifier", param.c_str());
            modifier = Parameter::PARAMETER_ANNOTATION_OUT;
            continue;
        }
        if(i == "inout")
        {
            DCHECK(modifier == Parameter::PARAMETER_ANNOTATION_DEFAULT, "Conflicts found in parameter(%s)'s qualifier", param.c_str());
            modifier = Parameter::PARAMETER_ANNOTATION_INOUT;
            continue;
        }
        if(i.startsWith("divisor("))
        {
            constexpr size_t pBegin = array_size("divisor(");
            const auto pEnd = i.find(')', pBegin);
            ASSERT(pEnd != String::npos);
            divisor = std::stoi(i.substr(pBegin, pEnd).c_str());
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
    return Parameter(std::move(type), std::move(name), static_cast<Parameter::Annotation>(modifier == Parameter::PARAMETER_ANNOTATION_DEFAULT ? Parameter::PARAMETER_ANNOTATION_IN : modifier), divisor);
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

String ShaderPreprocessor::Function::genOutCall(const enums::ShaderStageBit preShaderStage, const enums::ShaderStageBit shaderStage) const
{
    StringBuffer sb;
    sb << "ark_main(";
    const auto begin = _args.begin();
    for(auto iter = begin; iter != _args.end(); ++iter)
    {
        if(iter != begin)
            sb << ", ";
        if(iter->_annotation & Parameter::PARAMETER_ANNOTATION_OUT)
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
        if(i._annotation & Parameter::PARAMETER_ANNOTATION_OUT && Strings::capitalizeFirst(i._name) == name)
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
        if(i._annotation & Parameter::PARAMETER_ANNOTATION_OUT)
            count ++;
    return count;
}

ShaderPreprocessor::DeclarationList::DeclarationList(Source& source, const String& descriptor)
    : _source(source), _descriptor(descriptor)
{
}

void ShaderPreprocessor::DeclarationList::declare(const String& type, const char* prefix, const String& name, const String& layout, const char* qualifier, const bool isFlat)
{
    if(!_vars.has(name))
    {
        sp<String> declared = sp<String>::make(Strings::sprintf("%s%s %s %s%s;", isFlat ? "flat " : "", qualifier ? qualifier : _descriptor.c_str(), type.c_str(), prefix, name.c_str()));
        if(layout)
            _source.push_back(sp<String>::make(Strings::sprintf("layout (%s) %s", layout.c_str(), declared->c_str())));
        else
            _source.push_back(declared);

        _vars.push_back(name, Declaration(name, type, 1, std::move(declared)));
    }
    else
        CHECK(_vars.at(name).type() == type, "Declared type \"\" and variable type \"\" mismatch", _vars.at(name).type().c_str(), type.c_str());
}

void ShaderPreprocessor::DeclarationList::clear()
{
    for(const Declaration& i : _vars.values())
        i.setSource("");
    _vars.clear();
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

ShaderPreprocessor::Source::Source(String code)
    : _lines{sp<String>::make(std::move(code))}
{
}

String ShaderPreprocessor::Source::str(const char endl) const
{
    StringBuffer sb;
    for(const auto& i : _lines)
        if(i && !i->empty())
        {
            sb << *i;
            if(endl)
                sb << endl;
        }
    return sb.str();
}

void ShaderPreprocessor::Source::push_front(sp<String> fragment)
{
    _lines.push_front(std::move(fragment));
}

void ShaderPreprocessor::Source::push_back(sp<String> fragment)
{
    _lines.push_back(std::move(fragment));
}

bool ShaderPreprocessor::Source::search(const std::regex& pattern, const std::function<bool (const std::smatch&)>& traveller) const
{
    for(const sp<String>& i : _lines)
        if(!i->search(pattern, traveller))
            return false;
    return true;
}

bool ShaderPreprocessor::Source::contains(const String& str) const
{
    for(const sp<String>& i : _lines)
        if(i->find(str) != String::npos)
            return true;
    return false;
}

void ShaderPreprocessor::Source::replace(const String& str, const String& replacment)
{
    for(const sp<String>& i : _lines)
        *i = i->replace(str, replacment);
}

void ShaderPreprocessor::Source::replace(const std::regex& regexp, const std::function<sp<String>(const std::smatch&)>& replacer)
{
    Vector<sp<String>> inserting;
    for(auto iter = _lines.begin(); iter != _lines.end(); ++iter)
    {
        if((*iter)->strip().startsWith("//"))
            continue;

        bool matched = false;
        inserting.clear();
        (*iter)->search(regexp, [&matched, &inserting, replacer](const std::smatch& match) {
            if(sp<String> replacement = replacer(match))
                inserting.push_back(std::move(replacement));
            matched = true;
            return true;
        }, [&inserting](const String& unmatch) {
            inserting.push_back(sp<String>::make(unmatch));
            return true;
        });

        if(matched || inserting.size() > 1)
        {
            for(const auto& i : inserting)
            {
                iter = _lines.insert(iter, i);
                ++iter;
            }
            iter = _lines.erase(iter);
            if(iter == _lines.end())
                break;
        }
    }
}

void ShaderPreprocessor::Source::insertBefore(const String& statement, const String& str)
{
    for(const sp<String>& i : _lines)
    {
        String& code = *i;
        String::size_type pos = code.find(statement);
        if(pos != String::npos)
            code.insert(pos, str);
    }
}

ShaderPreprocessor::Declaration::Declaration(const String& name, const String& type, uint32_t length, sp<String> source)
    : _name(name), _type(type), _length(length), _usage(RenderUtil::toAttributeLayoutType(_name, _type)), _source(std::move(source))
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

bool ShaderPreprocessor::Declaration::operator<(const Declaration& other) const
{
    const int32_t v1 = _usage == Attribute::USAGE_CUSTOM ? 1 : 0;
    const int32_t v2 = other._usage == Attribute::USAGE_CUSTOM ? 1 : 0;
    return v1 < v2;
}

const String& ShaderPreprocessor::Declaration::source() const
{
    return *_source;
}

void ShaderPreprocessor::Declaration::setSource(String source) const
{
    *_source = std::move(source);
}

ShaderPreprocessor::Parameter::Parameter()
    : _annotation(PARAMETER_ANNOTATION_DEFAULT), _divisor(0)
{
}

ShaderPreprocessor::Parameter::Parameter(String type, String name, const Annotation annotation, const uint32_t divisor)
    : _type(std::move(type)), _name(std::move(name)), _annotation(annotation), _divisor(divisor)
{
}

const char* ShaderPreprocessor::Parameter::getQualifierStr() const
{
    const char* qualifiers[] = {"in", "in", "out", "inout"};
    DASSERT(_annotation >= PARAMETER_ANNOTATION_DEFAULT && _annotation <= PARAMETER_ANNOTATION_INOUT);
    return qualifiers[_annotation];
}

}
