#include "renderer/base/pipeline_building_context.h"

#include <regex>

#include "core/base/string_buffer.h"
#include "core/epi/notifier.h"
#include "core/inf/flatable.h"
#include "core/util/strings.h"

#include "renderer/base/pipeline_layout.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/impl/snippet/snippet_linked_chain.h"

namespace ark {

namespace {

class AlignedFlatable : public Flatable {
public:
    AlignedFlatable(const sp<Flatable>& delegate, size_t alignedSize)
        : _delegate(delegate), _aligned_size(alignedSize) {
        DCHECK(_delegate->size() <= _aligned_size, "Alignment is lesser than delegate's size(%d)", _delegate->size());
    }

    virtual void flat(void* buf) override {
        _delegate->flat(buf);
    }

    virtual uint32_t size() override {
        return _aligned_size;
    }

    virtual bool update(uint64_t timestamp) override {
        return _delegate->update(timestamp);
    }

private:
    sp<Flatable> _delegate;

    size_t _aligned_size;
};

}

PipelineBuildingContext::PipelineBuildingContext()
    : _input(sp<PipelineInput>::make())
{
}

PipelineBuildingContext::PipelineBuildingContext(sp<String> vertex, sp<String> fragment)
    : PipelineBuildingContext()
{
    addStage(std::move(vertex), Shader::SHADER_STAGE_VERTEX, Shader::SHADER_STAGE_NONE);
    addStage(std::move(fragment), Shader::SHADER_STAGE_FRAGMENT, Shader::SHADER_STAGE_VERTEX);
}

void PipelineBuildingContext::loadManifest(const document& manifest, BeanFactory& factory, const Scope& args)
{
    loadPredefinedUniform(factory, args, manifest);
    loadPredefinedSampler(factory, args, manifest);
    loadPredefinedAttribute(manifest);
}

void PipelineBuildingContext::initialize()
{
    initializePipelines();

    ShaderPreprocessor& firstStage = _stages.begin()->second;

    for(const auto& i : firstStage._declaration_ins.vars().values())
        addInputAttribute(i.name(), i.type());

    for(const auto& i : firstStage._main_block->_ins)
        firstStage.inDeclare(i._type, Strings::capitalizeFirst(i._name));

    std::set<String> passThroughVars;
    const ShaderPreprocessor* prestage = nullptr;;
    for(auto iter = _stages.begin(); iter != _stages.end(); ++iter)
    {
        if(iter != _stages.begin())
            iter->second->linkPreStage(*prestage, passThroughVars);
        prestage = iter->second.get();
    }

    Table<String, String> attributes;

    for(auto iter = _stages.begin(); iter != _stages.end(); ++iter)
        if(iter != _stages.begin())
            for(const auto& i : iter->second->_declaration_ins.vars().values())
                if(!attributes.has(i.name()))
                    attributes.push_back(i.name(), i.type());

    for(const auto& i : _attributes)
        if(!attributes.has(i.first))
            attributes.push_back(i.first, i.second.declareType());

    std::vector<String> generated;
    for(const auto& i : attributes)
    {
        if(!firstStage._declaration_ins.has(i.first)
                && !firstStage._declaration_outs.has(i.first)
                && !firstStage._main_block->hasOutAttribute(i.first))
        {
            generated.push_back(i.first);
            addAttribute(i.first, i.second);
        }
    }

    for(auto iter : _input->streams())
        iter.second.align();

    //TODO: link all outputs to next stage's inputs
    ShaderPreprocessor& _fragment = getStage(Shader::SHADER_STAGE_FRAGMENT);
    for(const auto& i : attributes)
    {
        if(passThroughVars.find(i.first) != passThroughVars.end())
            _fragment.inDeclare(i.second, i.first);
    }

    for(const String& i : generated)
    {
        firstStage.inDeclare(attributes.at(i), i);
        if(passThroughVars.find(i) != passThroughVars.end())
        {
            firstStage.outDeclare(attributes.at(i), i);
            firstStage.addPreMainSource(Strings::sprintf("%s%s = %s%s;", firstStage.outVarPrefix(), i.c_str(), firstStage.inVarPrefix(), i.c_str()));
        }
    }

    for(const auto& i : firstStage._main_block->_outs)
        firstStage.outDeclare(i._type, Strings::capitalizeFirst(i._name));
}

void PipelineBuildingContext::setupUniforms()
{
    int32_t binding = 0;
    for(auto& i : _stages)
        i.second->setupUniforms(_uniforms, binding);
}

const std::map<Shader::Stage, op<ShaderPreprocessor> >& PipelineBuildingContext::stages() const
{
    return _stages;
}

void PipelineBuildingContext::addAttribute(const String& name, const String& type)
{
    //TODO: add attribute to specified stage
    Attribute& attr = addPredefinedAttribute(name, type, Shader::SHADER_STAGE_VERTEX);
    _input->addAttribute(name, attr);
}

void PipelineBuildingContext::addSnippet(const sp<Snippet>& snippet)
{
    DASSERT(snippet);
    _snippet = _snippet ? sp<Snippet>::make<SnippetLinkedChain>(_snippet, snippet) : snippet;
}

void PipelineBuildingContext::addUniform(const String& name, Uniform::Type type, uint32_t length, const sp<Flatable>& flatable, int32_t binding)
{
    _uniforms.push_back(name, sp<Uniform>::make(name, type, length, flatable, binding));
}

void PipelineBuildingContext::addUniform(const sp<Uniform>& uniform)
{
    _uniforms.push_back(uniform->name(), uniform);
}

void PipelineBuildingContext::addInputAttribute(const String& name, const String& type)
{
    if(_input_vars.find(name) == _input_vars.end())
    {
        _input_vars.insert(name);
        addAttribute(name, type);
    }
}

Attribute& PipelineBuildingContext::addPredefinedAttribute(const String& name, const String& type, Shader::Stage stage)
{
    if(_attributes.find(name) == _attributes.end())
        _attributes[name] = makePredefinedAttribute(name, type);

    getStage(stage)->_predefined_parameters.push_back(ShaderPreprocessor::Parameter(type, name, ShaderPreprocessor::Parameter::PARAMETER_MODIFIER_IN));
    return _attributes[name];
}

const op<ShaderPreprocessor>& PipelineBuildingContext::getStage(Shader::Stage shaderStage) const
{
    auto iter = _stages.find(shaderStage);
    DCHECK(iter != _stages.end(), "Stage '%d' not found", shaderStage);
    return iter->second;
}

const op<ShaderPreprocessor>& PipelineBuildingContext::addStage(sp<String> source, Shader::Stage shaderStage, Shader::Stage preShaderStage)
{
    op<ShaderPreprocessor>& stage = _stages[shaderStage];
    DCHECK(!stage, "Stage '%d' has been initialized already", shaderStage);
    stage.reset(new ShaderPreprocessor(std::move(source), shaderStage, preShaderStage));
    return stage;
}

void PipelineBuildingContext::loadPredefinedAttribute(const document& manifest)
{
    for(const document& i : manifest->children("attribute"))
    {
        const String& name = Documents::ensureAttribute(i, Constants::Attributes::NAME);
        DCHECK(!name.empty(), "Empty name");
        DWARN(isupper(name[0]) || name.startsWith("a_"), "Attribute name \"%s\" should be capital first or started with a_", name.c_str());
        const String attrName = name.startsWith("a_") ? name.substr(2) : name;
        const String& type = Documents::ensureAttribute(i, Constants::Attributes::TYPE);
        uint32_t divisor = Documents::getAttribute<uint32_t>(i, "divisor", 0);
        addPredefinedAttribute(attrName, type, Shader::SHADER_STAGE_VERTEX).setDivisor(divisor);
    }
}

void PipelineBuildingContext::loadPredefinedUniform(BeanFactory& factory, const Scope& args, const document& manifest)
{
    for(const document& i : manifest->children("uniform"))
    {
        const String& name = Documents::ensureAttribute(i, Constants::Attributes::NAME);
        const String& type = Documents::ensureAttribute(i, Constants::Attributes::TYPE);
        const String& value = Documents::ensureAttribute(i, Constants::Attributes::VALUE);
        int32_t binding = Documents::getAttribute<int32_t>(i, Constants::Attributes::BINDING, -1);
        const sp<Flatable> flatable = factory.ensure<Flatable>(type, value, args);
        const uint32_t size = flatable->size();
        Uniform::Type uType = Uniform::TYPE_F1;
        switch (size) {
        case 4:
            if(type[0] == 'f')
                uType = Uniform::TYPE_F1;
            else if(type[0] == 'i')
                uType = Uniform::TYPE_I1;
            else
                FATAL("Unknow type \"%s\"", type.c_str());
            break;
        case 8:
            uType = Uniform::TYPE_F2;
            break;
        case 12:
            uType = Uniform::TYPE_F3;
            break;
        case 16:
            uType = Uniform::TYPE_F4;
            break;
        case 64:
            uType = Uniform::TYPE_MAT4;
            break;
        default:
            FATAL("Unknow type \"%s\"", type.c_str());
        }
        addUniform(name, uType, 1, uType == Uniform::TYPE_F3 ? sp<Flatable>::make<AlignedFlatable>(flatable, 16) : flatable, binding);
    }
}

void PipelineBuildingContext::loadPredefinedSampler(BeanFactory& factory, const Scope& args, const document& manifest)
{
    uint32_t binding = 0;

    for(const document& i : manifest->children("sampler"))
    {
        String name = Documents::getAttribute(i, Constants::Attributes::NAME);
        const sp<Texture> texture = factory.ensure<Texture>(i, args);
        if(!name)
            name = Strings::sprintf("u_Texture%d", binding);
        DCHECK(!_samplers.has(name), "Sampler \"%s\" redefined", name.c_str());
        _samplers.push_back(name, texture);
        binding++;
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
    if(type == "int")
        return Attribute("a_" + name, Attribute::TYPE_INTEGER, type, 1, false);
    if(type == "vec4")
        return Attribute("a_" + name, Attribute::TYPE_FLOAT, type, 4, false);
    if(type == "vec4b")
        return Attribute("a_" + name, Attribute::TYPE_UBYTE, type, 4, true);
    if(type == "vec3b")
        return Attribute("a_" + name, Attribute::TYPE_UBYTE, type, 3, true);
    if(type == "uint8")
        return Attribute("a_" + name, Attribute::TYPE_UBYTE, type, 1, false);
    if(type == "mat4")
        return Attribute("a_" + name, Attribute::TYPE_FLOAT, type, 16, false);
    DFATAL("Unknown attribute type \"%s\"", type.c_str());
    return Attribute();
}

void PipelineBuildingContext::initializePipelines()
{
    for(auto iter = _stages.begin(); iter != _stages.end(); ++iter)
        if(iter == _stages.begin())
            iter->second->initializeAsFirst(*this);
        else
            iter->second->initialize(*this);
}

}
