#include "renderer/base/pipeline_building_context.h"

#include <regex>

#include "core/base/string_buffer.h"
#include "core/inf/uploader.h"
#include "core/util/strings.h"

#include "renderer/base/pipeline_layout.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/base/snippet_delegate.h"
#include "renderer/impl/snippet/snippet_linked_chain.h"
#include "renderer/util/render_util.h"


namespace ark {

namespace {

class AlignedInput : public Uploader {
public:
    AlignedInput(sp<Uploader> delegate, size_t alignedSize)
        : Uploader(alignedSize), _delegate(std::move(delegate)), _aligned_size(alignedSize) {
        CHECK(_delegate->size() <= _aligned_size, "Alignment is lesser than delegate's size(%d)", _delegate->size());
    }

    virtual void upload(Writable& buf) override {
        _delegate->upload(buf);
    }

    virtual bool update(uint64_t timestamp) override {
        return _delegate->update(timestamp);
    }

private:
    sp<Uploader> _delegate;
    size_t _aligned_size;
};

}

PipelineBuildingContext::PipelineBuildingContext(const sp<RenderController>& renderController)
    : _render_controller(renderController), _input(sp<PipelineInput>::make())
{
}

PipelineBuildingContext::PipelineBuildingContext(const sp<RenderController>& renderController, sp<String> vertex, sp<String> fragment)
    : PipelineBuildingContext(renderController)
{
    addStage(std::move(vertex), PipelineInput::SHADER_STAGE_VERTEX, PipelineInput::SHADER_STAGE_NONE);
    addStage(std::move(fragment), PipelineInput::SHADER_STAGE_FRAGMENT, PipelineInput::SHADER_STAGE_VERTEX);
}

void PipelineBuildingContext::loadManifest(const document& manifest, BeanFactory& factory, const Scope& args)
{
    loadPredefinedUniform(factory, args, manifest);
    loadPredefinedSampler(factory, args, manifest);
    loadPredefinedImage(factory, args, manifest);
    loadPredefinedBuffer(factory, args, manifest);
    loadDefinitions(factory, args, manifest);
    loadPredefinedAttribute(manifest);
}

void PipelineBuildingContext::initialize()
{
    initializePipelines();

    ShaderPreprocessor& firstStage = _stages.begin()->second;

    for(const auto& i : firstStage._declaration_ins.vars().values())
        addInputAttribute(i.name(), i.type(), 0);

    for(const auto& [i, j] : _stages)
        for(const ShaderPreprocessor::Parameter& k : j->args())
            if(k._modifier & ShaderPreprocessor::Parameter::PARAMETER_MODIFIER_IN)
            {
                j->inDeclare(k._type, Strings::capitalizeFirst(k._name));
            }

    std::set<String> passThroughVars;
    const ShaderPreprocessor* prestage = nullptr;
    for(auto iter = _stages.begin(); iter != _stages.end(); ++iter)
    {
        if(iter != _stages.begin())
            iter->second->linkPreStage(*prestage, passThroughVars);
        prestage = iter->second.get();
    }

    Table<String, String> attributes;

    {
        auto iter = _stages.begin();
        for(++iter; iter != _stages.end(); ++iter)
            for(const auto& i : iter->second->_declaration_ins.vars().values())
                if(!attributes.has(i.name()))
                    attributes.push_back(i.name(), i.type());
    }

    for(const auto& i : _attributes)
        if(!attributes.has(i.first))
            attributes.push_back(i.first, i.second.declareType());

    std::vector<String> generated;
    for(const auto& [i, j] : attributes)
    {
        if(!firstStage._declaration_ins.has(i)
                && !firstStage._declaration_outs.has(i)
                && !firstStage._main_block->hasOutAttribute(i))
        {
            generated.push_back(i);
            addAttribute(i, j, 0);
        }
    }

    for(auto iter : _input->streams())
        iter.second.align();

    //TODO: link all outputs to next stage's inputs
    {
        auto iter = _stages.begin();
        for(++iter; iter != _stages.end(); ++iter)
            for(const auto& [i, j] : attributes)
            {
                if(passThroughVars.find(i) != passThroughVars.end())
                    iter->second->inDeclare(j, i);
            }
    }

    for(const String& i : generated)
    {
        firstStage.inDeclare(attributes.at(i), i);
        if(passThroughVars.find(i) != passThroughVars.end())
            firstStage.passThroughDeclare(attributes.at(i), i);
    }

    for(const auto& i : firstStage._main_block->_args)
        if(i._modifier == ShaderPreprocessor::Parameter::PARAMETER_MODIFIER_INOUT)
            firstStage.passThroughDeclare(i._type, Strings::capitalizeFirst(i._name));
        else if(i._modifier & ShaderPreprocessor::Parameter::PARAMETER_MODIFIER_OUT)
            firstStage.outDeclare(i._type, Strings::capitalizeFirst(i._name));

    Table<String, PipelineInput::SSBO> sobs;
    for(const auto& i : _stages)
    {
        for(const auto& [name, bindings] : i.second->_ssbos)
        {
            if(!sobs.has(name))
            {
                CHECK(_ssbos.has(name), "SSBO \"%s\" does not exist", name.c_str());
                sobs[name] = PipelineInput::SSBO(_ssbos.at(name), static_cast<uint32_t>(bindings));
            }
            sobs[name]._stages.insert(i.first);
        }
    }

    for(const auto& i : sobs)
        _input->ssbos().push_back(i.second);
}

void PipelineBuildingContext::setupUniforms()
{
    int32_t binding = 0;
    for(const auto& i : _stages)
        for(const auto& j : i.second->_ssbos)
            binding = std::max(binding, j.second + 1);

    for(auto& i : _stages)
        i.second->setupUniforms(_uniforms, binding);
}

const std::map<PipelineInput::ShaderStage, op<ShaderPreprocessor> >& PipelineBuildingContext::stages() const
{
    return _stages;
}

void PipelineBuildingContext::addAttribute(String name, String type, uint32_t divisor)
{
    //TODO: add attribute to specified stage
    const Attribute& attr = addPredefinedAttribute(name, type, divisor, PipelineInput::SHADER_STAGE_VERTEX);
    _input->addAttribute(std::move(name), attr);
}

void PipelineBuildingContext::addSnippet(const sp<Snippet>& snippet)
{
    DASSERT(snippet);
    _snippet = _snippet ? sp<Snippet>::make<SnippetLinkedChain>(_snippet, snippet) : snippet;
}

void PipelineBuildingContext::addUniform(String name, Uniform::Type type, uint32_t length, sp<Uploader> input, int32_t binding)
{
    addUniform(sp<Uniform>::make(std::move(name), type, length, std::move(input), binding));
}

void PipelineBuildingContext::addUniform(sp<Uniform> uniform)
{
    String name = uniform->name();
    _uniforms.push_back(std::move(name), std::move(uniform));
}

void PipelineBuildingContext::addInputAttribute(const String& name, const String& type, uint32_t divisor)
{
    if(_input_vars.find(name) == _input_vars.end())
    {
        _input_vars.insert(name);
        addAttribute(name, type, divisor);
    }
}

Attribute& PipelineBuildingContext::addPredefinedAttribute(const String& name, const String& type, uint32_t divisor, PipelineInput::ShaderStage stage)
{
    if(_attributes.find(name) == _attributes.end())
    {
        Attribute attr = makePredefinedAttribute(name, type);
        attr.setDivisor(divisor);
        _attributes[name] = std::move(attr);
    }

    getStage(stage)->_predefined_parameters.push_back(ShaderPreprocessor::Parameter(type, name, ShaderPreprocessor::Parameter::PARAMETER_MODIFIER_IN, divisor));
    return _attributes[name];
}

bool PipelineBuildingContext::hasStage(PipelineInput::ShaderStage shaderStage) const
{
    return _stages.find(shaderStage) != _stages.end();
}

ShaderPreprocessor* PipelineBuildingContext::tryGetStage(PipelineInput::ShaderStage shaderStage) const
{
    auto iter = _stages.find(shaderStage);
    return iter != _stages.end() ? iter->second.get() : nullptr;
}

const op<ShaderPreprocessor>& PipelineBuildingContext::getStage(PipelineInput::ShaderStage shaderStage) const
{
    auto iter = _stages.find(shaderStage);
    CHECK(iter != _stages.end(), "Stage '%d' not found", shaderStage);
    return iter->second;
}

const op<ShaderPreprocessor>& PipelineBuildingContext::addStage(sp<String> source, PipelineInput::ShaderStage shaderStage, PipelineInput::ShaderStage preShaderStage)
{
    op<ShaderPreprocessor>& stage = _stages[shaderStage];
    CHECK(!stage, "Stage '%d' has been initialized already", shaderStage);
    stage.reset(new ShaderPreprocessor(std::move(source), shaderStage, preShaderStage));
    return stage;
}

sp<Snippet> PipelineBuildingContext::makePipelineSnippet() const
{
    return sp<SnippetDelegate>::make(_snippet);
}

std::map<String, String> PipelineBuildingContext::toDefinitions() const
{
    std::map<String, String> definitions;
    for(const auto& [i, j] : _definitions)
        definitions.insert(std::make_pair(i, j->val()));
    return definitions;
}

void PipelineBuildingContext::loadPredefinedAttribute(const document& manifest)
{
    for(const document& i : manifest->children("attribute"))
    {
        const String& name = Documents::ensureAttribute(i, constants::NAME);
        DCHECK(!name.empty(), "Empty name");
        CHECK_WARN(isupper(name[0]) || name.startsWith("a_"), "Attribute name \"%s\" should be capital first or started with a_", name.c_str());
        const String attrName = name.startsWith("a_") ? name.substr(2) : name;
        const String& type = Documents::ensureAttribute(i, constants::TYPE);
        const uint32_t divisor = Documents::getAttribute<uint32_t>(i, "divisor", 0);
        addPredefinedAttribute(attrName, type, divisor, PipelineInput::SHADER_STAGE_VERTEX);
    }
}

void PipelineBuildingContext::loadPredefinedUniform(BeanFactory& factory, const Scope& args, const document& manifest)
{
    for(const document& i : manifest->children("uniform"))
    {
        const String& name = Documents::ensureAttribute(i, constants::NAME);
        const String& type = Documents::ensureAttribute(i, constants::TYPE);
        const String& value = Documents::ensureAttribute(i, constants::VALUE);
        int32_t binding = Documents::getAttribute<int32_t>(i, constants::BINDING, -1);
        sp<Builder<Uploader>> builder = factory.findBuilderByTypeValue<Uploader>(type, value);
        sp<Uploader> input = builder ? builder->build(args) : factory.ensure<Uploader>(value, args);
        uint32_t size = static_cast<uint32_t>(input->size());
        Uniform::Type uType = Uniform::toType(type);
        uint32_t componentSize = uType != Uniform::TYPE_STRUCT ? Uniform::getComponentSize(uType) : size;
        CHECK(componentSize, "Unknow type \"%s\"", type.c_str());
        addUniform(name, uType, size / componentSize, uType == Uniform::TYPE_F3 ? sp<Uploader>::make<AlignedInput>(input, 16) : input, binding);
    }
}

void PipelineBuildingContext::loadPredefinedSampler(BeanFactory& factory, const Scope& args, const document& manifest)
{
    uint32_t binding = 0;
    for(const document& i : manifest->children("sampler"))
    {
        String name = Documents::getAttribute(i, constants::NAME);
        sp<Texture> texture = factory.ensure<Texture>(i, args);
        if(!name)
            name = Strings::sprintf("u_Texture%d", binding);
        CHECK(!_samplers.has(name), "Sampler \"%s\" redefined", name.c_str());
        _samplers.push_back(std::move(name), std::move(texture));
        binding++;
    }
}

void PipelineBuildingContext::loadPredefinedImage(BeanFactory& factory, const Scope& args, const document& manifest)
{
    uint32_t binding = 0;
    for(const document& i : manifest->children("image"))
    {
        String name = Documents::getAttribute(i, constants::NAME);
        sp<Texture> texture = factory.ensure<Texture>(i, args);
        if(!name)
            name = Strings::sprintf("u_Image%d", binding);
        CHECK(!_images.has(name), "Image \"%s\" redefined", name.c_str());
        _images.push_back(std::move(name), std::move(texture));
        binding++;
    }
}

void PipelineBuildingContext::loadPredefinedBuffer(BeanFactory& factory, const Scope& args, const document& manifest)
{
    for(const document& i : manifest->children("buffer"))
    {
        String name = Documents::getAttribute(i, constants::NAME);
        CHECK(!_ssbos.has(name), "Buffer object \"%s\" redefined", name.c_str());
        _ssbos.push_back(name, factory.ensure<Buffer>(i, args));
    }
}

Attribute PipelineBuildingContext::makePredefinedAttribute(const String& name, const String& type)
{
    if(name == "TexCoordinate")
        return Attribute("a_TexCoordinate", Attribute::TYPE_USHORT, type, 2, true);
    if(name == "Position")
    {
        if(type == "int")
            return Attribute("a_Position", Attribute::TYPE_INTEGER, type, 1, false);
        CHECK(type == "vec2" || type == "vec3" || type == "vec4", "Unacceptable Position type: '%s', must be in [int, vec2, vec3, vec4]", type.c_str());
        return Attribute("a_Position", Attribute::TYPE_FLOAT, type, std::min<uint32_t>(3, static_cast<uint32_t>(type.at(3) - '0')), false);
    }
    return RenderUtil::makePredefinedAttribute("a_" + name, type);
}

void PipelineBuildingContext::initializePipelines()
{
    for(auto iter = _stages.begin(); iter != _stages.end(); ++iter)
        if(iter == _stages.begin())
            iter->second->initializeAsFirst(*this);
        else
            iter->second->initialize(*this);
}

void PipelineBuildingContext::loadDefinitions(BeanFactory& factory, const Scope& args, const document& manifest)
{
    for(const document& i : manifest->children("define"))
    {
        String name = Documents::getAttribute(i, constants::NAME);
        CHECK_WARN(_definitions.find(name) == _definitions.end(), "Definition \"%s\" redefined", name.c_str());
        _definitions.insert(std::make_pair(name, factory.ensureBuilder<StringVar>(i, constants::VALUE)->build(args)));
    }
}

}
