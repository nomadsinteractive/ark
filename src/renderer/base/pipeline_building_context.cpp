#include "renderer/base/pipeline_building_context.h"

#include <regex>

#include "core/inf/uploader.h"
#include "core/util/strings.h"

#include "renderer/base/pipeline_configuration.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/impl/snippet/snippet_composite.h"
#include "renderer/inf/renderer_factory.h"
#include "renderer/util/render_util.h"


namespace ark {

namespace {

class AlignedUploader final : public Uploader {
public:
    AlignedUploader(sp<Uploader> delegate, size_t alignedSize)
        : Uploader(alignedSize), _delegate(std::move(delegate)), _aligned_size(alignedSize) {
        CHECK(_delegate->size() <= _aligned_size, "Alignment is lesser than delegate's size(%d)", _delegate->size());
    }

    void upload(Writable& buf) override {
        _delegate->upload(buf);
    }

    bool update(const uint64_t timestamp) override {
        return _delegate->update(timestamp);
    }

private:
    sp<Uploader> _delegate;
    size_t _aligned_size;
};

Attribute::Usage toAttributeLayoutType(const String& name, const String& type)
{
    if(name.startsWith("uv") || name.startsWith("texcoordinate"))
    {
        CHECK(type == "int" || type == "vec2" || type == "vec3", "Unacceptable TexCoordinate type: '%s', must be in [int, vec2, vec3]", type.c_str());
        return Attribute::USAGE_TEX_COORD;
    }
    if(name.startsWith("position"))
    {
        CHECK(type == "int" || type == "vec2" || type == "vec3" || type == "vec4", "Unacceptable Position type: '%s', must be in [int, vec2, vec3, vec4]", type.c_str());
        return Attribute::USAGE_POSITION;
    }
    if(name.startsWith("color"))
    {
        CHECK(type == "int" || type == "vec3" || type == "vec4"|| type == "vec3b" || type == "vec4b", "Unacceptable Color type: '%s', must be in [int, vec3, vec4, vec3b, vec4b]", type.c_str());
        return Attribute::USAGE_COLOR;
    }
    if(name == "model")
    {
        CHECK(type == "mat4", "Unacceptable Model type: '%s', must be in [mat4]", type.c_str());
        return Attribute::USAGE_MODEL_MATRIX;
    }
    if(name == "normal")
    {
        CHECK(type == "vec3", "Unacceptable Normal type: '%s', must be in [vec3]", type.c_str());
        return Attribute::USAGE_NORMAL;
    }
    if(name == "tangent")
    {
        CHECK(type == "vec3", "Unacceptable Tangent type: '%s', must be in [vec3]", type.c_str());
        return Attribute::USAGE_TANGENT;
    }
    if(name == "bitangent")
    {
        CHECK(type == "vec3", "Unacceptable Bitangent type: '%s', must be in [vec3]", type.c_str());
        return Attribute::USAGE_BITANGENT;
    }
    return Attribute::USAGE_CUSTOM;
}

Attribute makePredefinedAttribute(const String& name, const String& type)
{
    const Attribute::Usage layoutType = toAttributeLayoutType(name.toLower(), type);

    if(layoutType == Attribute::USAGE_TEX_COORD)
        return {Attribute::USAGE_TEX_COORD, Strings::sprintf("a_%s", name.c_str()), Attribute::TYPE_USHORT, type, 2, true};

    if(layoutType == Attribute::USAGE_POSITION)
    {
        if(type == "int")
            return {Attribute::USAGE_POSITION, "a_Position", Attribute::TYPE_INT, type, 1, false};
        CHECK(type == "vec2" || type == "vec3" || type == "vec4", "Unacceptable Position type: '%s', must be in [int, vec2, vec3, vec4]", type.c_str());
        return {Attribute::USAGE_POSITION, "a_Position", Attribute::TYPE_FLOAT, type, std::min<uint32_t>(3, static_cast<uint32_t>(type.at(3) - '0')), false};
    }
    return RenderUtil::makePredefinedAttribute("a_" + name, type, layoutType);
}

}

PipelineBuildingContext::PipelineBuildingContext()
    : _pipeline_layout(sp<PipelineLayout>::make())
{
}

PipelineBuildingContext::PipelineBuildingContext(sp<String> vertex, sp<String> fragment)
    : PipelineBuildingContext()
{
    addStage(std::move(vertex), nullptr, Enum::SHADER_STAGE_BIT_VERTEX, Enum::SHADER_STAGE_BIT_NONE);
    addStage(std::move(fragment), nullptr, Enum::SHADER_STAGE_BIT_FRAGMENT, Enum::SHADER_STAGE_BIT_VERTEX);
}

void PipelineBuildingContext::loadManifest(const document& manifest, BeanFactory& factory, const Scope& args)
{
    loadPredefinedUniform(factory, args, manifest);
    loadPredefinedSampler(factory, args, manifest);
    loadPredefinedImage(factory, args, manifest);
    loadLayoutBindings(factory, args, manifest);
    loadPredefinedBuffer(factory, args, manifest);
    loadDefinitions(factory, args, manifest);
    loadPredefinedAttribute(manifest);
}

void PipelineBuildingContext::initializeAttributes()
{
    if(_rendering_stages.empty())
        return;

    ShaderPreprocessor& firstStage = _rendering_stages.begin()->second;

    for(const auto& i : firstStage._declaration_ins.vars().values())
        addInputAttribute(i.name(), i.type(), 0);

    for(const auto& [i, j] : _rendering_stages)
        for(const ShaderPreprocessor::Parameter& k : j->args())
            if(k._annotation & ShaderPreprocessor::Parameter::PARAMETER_ANNOTATION_IN)
                j->inDeclare(k._type, Strings::capitalizeFirst(k._name));

    std::set<String> passThroughVars;
    const ShaderPreprocessor* prestage = nullptr;
    for(const auto& [k, v] : _rendering_stages)
    {
        if(prestage)
            v->linkPreStage(*prestage, passThroughVars);
        prestage = v.get();
    }

    Table<String, String> attributes;
    {
        for(auto iter = std::next(_rendering_stages.begin()); iter != _rendering_stages.end(); ++iter)
            for(const auto& i : iter->second->_declaration_ins.vars().values())
                if(!attributes.has(i.name()))
                    attributes.push_back(i.name(), i.type());
    }

    for(const auto& [k, v] : _attributes)
        if(!attributes.has(k))
            attributes.push_back(k, v.declareType());

    Vector<String> generated;
    for(const auto& [k, v] : attributes)
        if(!firstStage._declaration_ins.has(k)
           && !firstStage._declaration_outs.has(k)
           && !firstStage._main_block->hasOutAttribute(k))
        {
            generated.push_back(k);
            addAttribute(k, v, 0);
        }

    const uint32_t alignment = Ark::instance().renderController()->renderEngine()->rendererFactory()->features()._attribute_alignment;
    for(auto &[k, v] : _pipeline_layout->streamLayouts())
        v.align(k == 0 ? 4 : alignment);

    //TODO: link all outputs to next stage's inputs
    {
        for(auto iter = std::next(_rendering_stages.begin()); iter != _rendering_stages.end(); ++iter)
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
        if(i._annotation == ShaderPreprocessor::Parameter::PARAMETER_ANNOTATION_INOUT)
            firstStage.passThroughDeclare(i._type, Strings::capitalizeFirst(i._name));
        else if(i._annotation & ShaderPreprocessor::Parameter::PARAMETER_ANNOTATION_OUT)
            firstStage.outDeclare(i._type, Strings::capitalizeFirst(i._name));
}

void PipelineBuildingContext::initializeSSBO() const
{
    Table<String, PipelineLayout::SSBO> sobs;
    for(const ShaderPreprocessor* preprocessor : _stages)
        for(const auto& [name, bindings] : preprocessor->_ssbos)
        {
            if(!sobs.has(name))
            {
                CHECK(_ssbos.has(name), "SSBO \"%s\" does not exist", name.c_str());
                sobs[name] = PipelineLayout::SSBO(_ssbos.at(name), bindings);
            }
            sobs[name]._stages.set(preprocessor->_shader_stage);
        }

    _pipeline_layout->_ssbos = std::move(sobs.values());
}

void PipelineBuildingContext::tryBindCamera(const ShaderPreprocessor& shaderPreprocessor, const Camera& camera)
{
    tryBindUniformMatrix(shaderPreprocessor, "u_VP", camera.vp());
    tryBindUniformMatrix(shaderPreprocessor, "u_View", camera.view());
    tryBindUniformMatrix(shaderPreprocessor, "u_Projection", camera.projection());
}

void PipelineBuildingContext::tryBindUniformMatrix(const ShaderPreprocessor& shaderPreprocessor, String name, const sp<Mat4>& matrix)
{
    if(sp<Uniform> uniform = shaderPreprocessor.makeUniformInput(std::move(name), Uniform::TYPE_MAT4))
    {
        uniform->setUploader(sp<Uploader>::make<UploaderOfVariable<M4>>(matrix));
        addUniform(std::move(uniform));
    }
}

void PipelineBuildingContext::initialize(const Camera& camera)
{
    initializeStages(camera);
    initializeSSBO();
    initializeAttributes();
    initializeUniforms();
}

void PipelineBuildingContext::initializeUniforms()
{
    for(ShaderPreprocessor* i : _stages)
        i->setupUniforms(_uniforms);

    int32_t binding = 0;
    for(const ShaderPreprocessor* stage : _stages)
        if(const Vector<String>& uniformNames = stage->_declaration_uniforms.vars().keys(); !uniformNames.empty())
        {
            const Set<String> uniformNameSet(uniformNames.begin(), uniformNames.end());
            HashId hash = 0;
            for(const String& i : uniformNameSet)
                hash += hash * 101 + i.hash();

            sp<PipelineLayout::UBO>& ubo = _ubos[hash];
            if(ubo == nullptr)
                ubo = sp<PipelineLayout::UBO>::make(binding++);
            ubo->_stages.set(stage->_shader_stage);
            for(const String& i : uniformNames)
                ubo->addUniform(_uniforms.at(i));
        }
}

const Vector<ShaderPreprocessor*>& PipelineBuildingContext::stages() const
{
    return _stages;
}

const Map<Enum::ShaderStageBit, op<ShaderPreprocessor>>& PipelineBuildingContext::renderStages() const
{
    return _rendering_stages;
}

const op<ShaderPreprocessor>& PipelineBuildingContext::computingStage() const
{
    return _computing_stage;
}

void PipelineBuildingContext::addAttribute(String name, String type, const uint32_t divisor)
{
    //TODO: add attribute to specified stage
    const Attribute& attr = addPredefinedAttribute(name, std::move(type), divisor, Enum::SHADER_STAGE_BIT_VERTEX);
    _pipeline_layout->addAttribute(std::move(name), attr);
}

void PipelineBuildingContext::addUniform(String name, Uniform::Type type, uint32_t length, sp<Uploader> input)
{
    addUniform(sp<Uniform>::make(std::move(name), type, length, std::move(input)));
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

Attribute& PipelineBuildingContext::addPredefinedAttribute(const String& name, const String& type, const uint32_t divisor, Enum::ShaderStageBit stage)
{
    if(_attributes.find(name) == _attributes.end())
    {
        Attribute attr = makePredefinedAttribute(name, type);
        attr.setDivisor(divisor);
        _attributes[name] = std::move(attr);
    }

    getRenderStage(stage)->_predefined_parameters.push_back(ShaderPreprocessor::Parameter(type, name, ShaderPreprocessor::Parameter::PARAMETER_ANNOTATION_IN, divisor));
    return _attributes[name];
}

ShaderPreprocessor* PipelineBuildingContext::tryGetRenderStage(Enum::ShaderStageBit shaderStage) const
{
    const auto iter = _rendering_stages.find(shaderStage);
    return iter != _rendering_stages.end() ? iter->second.get() : nullptr;
}

const op<ShaderPreprocessor>& PipelineBuildingContext::getRenderStage(Enum::ShaderStageBit shaderStage) const
{
    const auto iter = _rendering_stages.find(shaderStage);
    CHECK(iter != _rendering_stages.end(), "Stage '%d' not found", shaderStage);
    return iter->second;
}

const op<ShaderPreprocessor>& PipelineBuildingContext::addStage(sp<String> source, document manifest, const Enum::ShaderStageBit shaderStage, const Enum::ShaderStageBit preShaderStage)
{
    op<ShaderPreprocessor>& stage = shaderStage == Enum::SHADER_STAGE_BIT_COMPUTE ? _computing_stage : _rendering_stages[shaderStage];
    CHECK(!stage, "Stage '%d' has been initialized already", shaderStage);
    stage.reset(new ShaderPreprocessor(std::move(source), std::move(manifest), shaderStage, preShaderStage));
    _stages.emplace_back(stage.get());
    return stage;
}

Map<String, String> PipelineBuildingContext::toDefinitions() const
{
    Map<String, String> definitions;
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
        addPredefinedAttribute(attrName, type, divisor, Enum::SHADER_STAGE_BIT_VERTEX);
    }
}

void PipelineBuildingContext::loadPredefinedUniform(BeanFactory& factory, const Scope& args, const document& manifest)
{
    for(const document& i : manifest->children("uniform"))
    {
        const String& name = Documents::ensureAttribute(i, constants::NAME);
        const String& type = Documents::ensureAttribute(i, constants::TYPE);
        const String& value = Documents::ensureAttribute(i, constants::VALUE);
        const builder<Uploader> builder = factory.findBuilderByTypeValue<Uploader>(type, value);
        sp<Uploader> uploader = builder ? builder->build(args) : factory.ensure<Uploader>(value, args);
        const uint32_t size = static_cast<uint32_t>(uploader->size());
        const Uniform::Type uType = Uniform::toType(type);
        const uint32_t componentSize = uType != Uniform::TYPE_STRUCT ? Uniform::getComponentSize(uType) : size;
        CHECK(componentSize, "Unknow type \"%s\"", type.c_str());
        addUniform(sp<Uniform>::make(std::move(name), uType, componentSize, size / componentSize, uType == Uniform::TYPE_F3 ? sp<Uploader>::make<AlignedUploader>(std::move(uploader), 16) : std::move(uploader)));
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
        CHECK(texture->usage().has(Texture::USAGE_STORAGE), "Texture \"%s\" declared as a storage image here but its usage has no \"storage\" bits", name.c_str());
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

void PipelineBuildingContext::loadLayoutBindings(BeanFactory& factory, const Scope& args, const document& manifest)
{
    for(const document& i : manifest->children(constants::LAYOUT))
    {
        const int32_t binding = Documents::getAttribute(i, "binding", -1);
        String name = Documents::getAttribute(i, constants::NAME);
        CHECK(name != "" || binding != -1, "Pipeline layout should have either name or binding defined: %s", Documents::toString(i).c_str());
        const LayoutBindingType type = Documents::getAttribute(i, constants::TYPE, LAYOUT_BINDING_TYPE_AUTO);
        if(type == LAYOUT_BINDING_TYPE_AUTO)
        {
            FATAL("LAYOUT_BINDING_TYPE_AUTO Unimplemented");
        }
        const Texture::Usage usage = Documents::getAttribute<Texture::Usage>(i, "usage", {Texture::USAGE_AUTO});
        _layout_bindings.push_back({type, factory.ensure<Texture>(i, args), usage, std::move(name), binding});
    }
}

void PipelineBuildingContext::initializeStages(const Camera& camera)
{
    for(ShaderPreprocessor* preprocessor : _stages)
        if(preprocessor->_shader_stage == Enum::SHADER_STAGE_BIT_VERTEX)
            preprocessor->initializeAsFirst(*this);
        else
            preprocessor->initialize(*this);

    if(const ShaderPreprocessor* vertex = tryGetRenderStage(Enum::SHADER_STAGE_BIT_VERTEX))
        tryBindCamera(*vertex, camera);

    uint32_t binding = 0;
    Table<String, PipelineLayout::DescriptorSet>& samplers = _pipeline_layout->_samplers;
    Table<String, PipelineLayout::DescriptorSet>& images = _pipeline_layout->_images;
    {
        if(const ShaderPreprocessor* vertex = tryGetRenderStage(Enum::SHADER_STAGE_BIT_VERTEX))
        {
            for(const String& i : vertex->_declaration_images.vars().keys())
                binding = images[i].addStage(Enum::SHADER_STAGE_BIT_VERTEX, binding);
        }
        if(const ShaderPreprocessor* fragment = tryGetRenderStage(Enum::SHADER_STAGE_BIT_FRAGMENT))
        {
            for(const String& i : fragment->_declaration_samplers.vars().keys())
                binding = samplers[i].addStage(Enum::SHADER_STAGE_BIT_FRAGMENT, binding);
            for(const String& i : fragment->_declaration_images.vars().keys())
                binding = images[i].addStage(Enum::SHADER_STAGE_BIT_FRAGMENT, binding);
        }
    }

    if(const ShaderPreprocessor* compute = _computing_stage.get())
    {
        tryBindCamera(*compute, camera);
        for(const String& i : compute->_declaration_samplers.vars().keys())
            binding = samplers[i].addStage(Enum::SHADER_STAGE_BIT_COMPUTE, binding);
        for(const String& i : compute->_declaration_images.vars().keys())
            binding = images[i].addStage(Enum::SHADER_STAGE_BIT_COMPUTE, binding);
    }
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

template<> PipelineBuildingContext::LayoutBindingType StringConvert::eval<PipelineBuildingContext::LayoutBindingType>(const String& repr)
{
    if(repr == "image")
        return PipelineBuildingContext::LAYOUT_BINDING_TYPE_IMAGE;
    if(repr == "sampler")
        return PipelineBuildingContext::LAYOUT_BINDING_TYPE_SAMPLER;
    if(repr == "ssbo")
        return PipelineBuildingContext::LAYOUT_BINDING_TYPE_SSBO;
    if(repr == "ubo")
        return PipelineBuildingContext::LAYOUT_BINDING_TYPE_UBO;
    CHECK(repr == "auto", "Unknow LayoutBindingType: \"%s, supported values are [\"image\", \"sampler\", \"ssbo\", \"ubo\", ...]", repr.c_str());
    return PipelineBuildingContext::LAYOUT_BINDING_TYPE_AUTO;
}

}
