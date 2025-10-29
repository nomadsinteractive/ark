#include "renderer/base/pipeline_building_context.h"

#include <ranges>
#include <regex>

#include "core/inf/uploader.h"
#include "core/util/strings.h"

#include "renderer/base/render_engine_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/impl/snippet/snippet_composite.h"
#include "renderer/inf/renderer_factory.h"
#include "renderer/util/render_util.h"


namespace ark {

namespace {

Attribute makePredefinedAttribute(const String& name, const String& type)
{
    const Attribute::Usage layoutType = RenderUtil::toAttributeLayoutType(name, type);

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

bool checkUnsuedUniforms(const Table<String, sp<Uniform>>& uniforms, const Vector<ShaderPreprocessor*>& stages)
{
    for(const String& i : uniforms.keys())
    {
        bool contains = false;
        for(const ShaderPreprocessor* stage : stages)
            if(stage->_declaration_uniforms.has(i))
            {
                contains = true;
                break;
            }
        DCHECK(contains, "Uniform \"%s\" declared in manifest but it is not used in all stages", i.c_str());
    }
    return true;
}

sp<Uniform> loadUniform(BeanFactory& factory, const Scope& args, const String& name, const String& type, const String& value)
{
    const sp<Builder<Uploader>> builder = factory.ensureBuilderByTypeValue<sp<Uploader>>(type, value);
    sp<Uploader> uploader = builder->build(args);
    const uint32_t size = static_cast<uint32_t>(uploader->size());
    const Uniform::Type uType = Uniform::toType(type);
    const uint32_t componentSize = uType != Uniform::TYPE_STRUCT ? Uniform::getComponentSize(uType) : size;
    CHECK(componentSize, "Unknow type \"%s\"", type.c_str());
    CHECK_WARN(uType != Uniform::TYPE_F3, "We strongly against declaring vec3 uniform type for \"%s\"", name.c_str());
    return sp<Uniform>::make(std::move(name), uType, componentSize, size / componentSize, std::move(uploader));
}

}

PipelineBuildingContext::PipelineBuildingContext(BeanFactory factory, const Scope& args)
    : _pipeline_layout(sp<PipelineLayout>::make()), _factory(std::move(factory)), _args(args)
{
}

void PipelineBuildingContext::loadManifest(const document& manifest)
{
    loadPredefinedUniform(manifest);
    loadPredefinedSampler(manifest);
    loadPredefinedImage(manifest);
    loadPredefinedBuffer(manifest);
    loadDefinitions(manifest);
    loadPredefinedAttribute(manifest);
}

void PipelineBuildingContext::initializeAttributes()
{
    if(_rendering_stages.empty())
        return;

    ShaderPreprocessor& firstStage = _rendering_stages.begin()->second;

    // for(const auto& i : firstStage._declaration_ins.vars().values())
    //     addInputAttribute(i.name(), i.type(), 0);

    for(const auto& v : std::views::values(_rendering_stages))
        for(const ShaderPreprocessor::Parameter& i : v->args())
            if(i._annotation & ShaderPreprocessor::Parameter::PARAMETER_ANNOTATION_IN)
                v->inDeclare(i._type, Strings::capitalizeFirst(i._name));

    Set<String> passThroughVars;
    const ShaderPreprocessor* prestage = nullptr;
    for(const auto& v : std::views::values(_rendering_stages))
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

    Vector<VertexAttribute> vertexAttributes = _vertex_attributes.values();
    std::stable_sort(vertexAttributes.begin(), vertexAttributes.end());
    for(VertexAttribute& i : vertexAttributes)
        doAddVertexAttribute(std::move(i._name), std::move(i._type), i._divisor);

    const uint32_t alignment = Ark::instance().renderController()->renderEngine()->rendererFactory()->features()._attribute_alignment;
    _pipeline_layout->setStreamLayoutAlignment(alignment);

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
        if(passThroughVars.contains(i))
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
    if(sp<Uniform> uniform = shaderPreprocessor.makeUniform(std::move(name), Uniform::TYPE_MAT4))
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

    _pipeline_layout->initialize(*this);
}

void PipelineBuildingContext::initializeUniforms()
{
    for(ShaderPreprocessor* i : _stages)
    {
        for(const auto& [k, v] : i->_declaration_uniforms.vars())
            if(!_uniforms.has(k))
                addUniform(loadUniform(_factory, _args, k, v.type(), "$" + k));

        for(const auto& [k, v] : i->_declaration_samplers.vars())
            if(!_samplers.has(k))
                if(Optional<sp<Texture>> sampler = _args.getObject<sp<Texture>>(k))
                    _samplers.push_back(k, std::move(sampler.value()));

        for(const auto& [k, v] : i->_declaration_images.vars())
            if(!_images.has(k))
                if(Optional<sp<Texture>> image = _args.getObject<sp<Texture>>(k))
                    _images.push_back(k, std::move(image.value()));
    }

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
    DCHECK(checkUnsuedUniforms(_uniforms, _stages), "");
}

const Vector<ShaderPreprocessor*>& PipelineBuildingContext::stages() const
{
    return _stages;
}

const Map<enums::ShaderStageBit, op<ShaderPreprocessor>>& PipelineBuildingContext::renderStages() const
{
    return _rendering_stages;
}

const op<ShaderPreprocessor>& PipelineBuildingContext::computingStage() const
{
    return _computing_stage;
}

void PipelineBuildingContext::doAddVertexAttribute(String name, String type, const uint32_t divisor)
{
    //TODO: add attribute to specified stage
    const Attribute& attr = addPredefinedAttribute(name, std::move(type), divisor, enums::SHADER_STAGE_BIT_VERTEX);
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

void PipelineBuildingContext::addAttribute(const String& name, const String& type, const uint32_t divisor)
{
    if(_vertex_attributes.find(name) == _vertex_attributes.end())
        _vertex_attributes.push_back(name, {name, type, divisor});
}

Attribute& PipelineBuildingContext::addPredefinedAttribute(const String& name, const String& type, const uint32_t divisor, const enums::ShaderStageBit stage)
{
    if(!_attributes.contains(name))
    {
        Attribute attr = makePredefinedAttribute(name, type);
        attr.setDivisor(divisor);
        _attributes[name] = std::move(attr);
    }

    getRenderStage(stage)->_predefined_parameters.push_back(ShaderPreprocessor::Parameter(type, name, ShaderPreprocessor::Parameter::PARAMETER_ANNOTATION_IN, divisor));
    return _attributes[name];
}

ShaderPreprocessor* PipelineBuildingContext::tryGetRenderStage(enums::ShaderStageBit shaderStage) const
{
    const auto iter = _rendering_stages.find(shaderStage);
    return iter != _rendering_stages.end() ? iter->second.get() : nullptr;
}

const op<ShaderPreprocessor>& PipelineBuildingContext::getRenderStage(enums::ShaderStageBit shaderStage) const
{
    const auto iter = _rendering_stages.find(shaderStage);
    CHECK(iter != _rendering_stages.end(), "Stage '%d' not found", shaderStage);
    return iter->second;
}

const op<ShaderPreprocessor>& PipelineBuildingContext::addStage(String source, document manifest, const enums::ShaderStageBit shaderStage, const enums::ShaderStageBit preShaderStage)
{
    op<ShaderPreprocessor>& stage = shaderStage == enums::SHADER_STAGE_BIT_COMPUTE ? _computing_stage : _rendering_stages[shaderStage];
    CHECK(!stage, "Stage '%d' has been initialized already", shaderStage);
    stage.reset(new ShaderPreprocessor(std::move(source), std::move(manifest), shaderStage, preShaderStage));
    _stages.emplace_back(stage.get());
    return stage;
}

Map<String, String> PipelineBuildingContext::toDefinitions() const
{
    Map<String, String> definitions;
    for(const auto& [i, j] : _definitions)
        definitions.insert(std::make_pair(i, j->val().data()));
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
        addPredefinedAttribute(attrName, type, divisor, enums::SHADER_STAGE_BIT_VERTEX);
    }
}

void PipelineBuildingContext::loadPredefinedUniform(const document& manifest)
{
    for(const document& i : manifest->children("uniform"))
    {
        const String& name = Documents::ensureAttribute(i, constants::NAME);
        const String& type = Documents::ensureAttribute(i, constants::TYPE);
        const String& value = Documents::ensureAttribute(i, constants::VALUE);
        addUniform(loadUniform(_factory, _args, name, type, value));
    }
}

void PipelineBuildingContext::loadPredefinedSampler(const document& manifest)
{
    uint32_t binding = 0;
    for(const document& i : manifest->children("sampler"))
    {
        String name = Documents::getAttribute(i, constants::NAME);
        sp<Texture> texture = _factory.ensure<Texture>(i, _args);
        if(!name)
            name = Strings::sprintf("u_Texture%d", binding);
        CHECK(!_samplers.has(name), "Sampler \"%s\" redefined", name.c_str());
        _samplers.push_back(std::move(name), std::move(texture));
        binding++;
    }
}

void PipelineBuildingContext::loadPredefinedImage(const document& manifest)
{
    uint32_t binding = 0;
    for(const document& i : manifest->children("image"))
    {
        String name = Documents::getAttribute(i, constants::NAME);
        sp<Texture> texture = _factory.ensure<Texture>(i, _args);
        if(!name)
            name = Strings::sprintf("u_Image%d", binding);
        CHECK(!_images.has(name), "Image \"%s\" redefined", name.c_str());
        CHECK(texture->usage().contains(Texture::USAGE_STORAGE), "Texture \"%s\" declared as a storage image here but its usage has no \"storage\" bits", name.c_str());
        _images.push_back(std::move(name), std::move(texture));
        binding++;
    }
}

void PipelineBuildingContext::loadPredefinedBuffer(const document& manifest)
{
    for(const document& i : manifest->children("buffer"))
    {
        String name = Documents::getAttribute(i, constants::NAME);
        CHECK(!_ssbos.has(name), "Buffer object \"%s\" redefined", name.c_str());
        _ssbos.push_back(name, _factory.ensure<Buffer>(i, _args));
    }
}

void PipelineBuildingContext::initializeStages(const Camera& camera)
{
    for(ShaderPreprocessor* preprocessor : _stages)
        if(preprocessor->_shader_stage == enums::SHADER_STAGE_BIT_VERTEX)
            preprocessor->initializeAsFirst(*this);
        else
            preprocessor->initialize(*this);

    if(const ShaderPreprocessor* vertex = tryGetRenderStage(enums::SHADER_STAGE_BIT_VERTEX))
        tryBindCamera(*vertex, camera);

    uint32_t binding = 0;
    Table<String, PipelineLayout::DescriptorSet>& samplers = _pipeline_layout->_samplers;
    Table<String, PipelineLayout::DescriptorSet>& images = _pipeline_layout->_images;
    {
        if(const ShaderPreprocessor* vertex = tryGetRenderStage(enums::SHADER_STAGE_BIT_VERTEX))
        {
            for(const String& i : vertex->_declaration_images.vars().keys())
                binding = images[i].addStage(enums::SHADER_STAGE_BIT_VERTEX, binding);
        }
        if(const ShaderPreprocessor* fragment = tryGetRenderStage(enums::SHADER_STAGE_BIT_FRAGMENT))
        {
            for(const String& i : fragment->_declaration_samplers.vars().keys())
                binding = samplers[i].addStage(enums::SHADER_STAGE_BIT_FRAGMENT, binding);
            for(const String& i : fragment->_declaration_images.vars().keys())
                binding = images[i].addStage(enums::SHADER_STAGE_BIT_FRAGMENT, binding);
        }
    }

    if(const ShaderPreprocessor* compute = _computing_stage.get())
    {
        tryBindCamera(*compute, camera);
        for(const String& i : compute->_declaration_samplers.vars().keys())
            binding = samplers[i].addStage(enums::SHADER_STAGE_BIT_COMPUTE, binding);
        for(const String& i : compute->_declaration_images.vars().keys())
            binding = images[i].addStage(enums::SHADER_STAGE_BIT_COMPUTE, binding);
    }
}

void PipelineBuildingContext::loadDefinitions(const document& manifest)
{
    for(const document& i : manifest->children("define"))
    {
        String name = Documents::getAttribute(i, constants::NAME);
        CHECK_WARN(!_definitions.contains(name), "Definition \"%s\" redefined", name.c_str());
        _definitions.insert(std::make_pair(name, _factory.ensureBuilder<StringVar>(i, constants::VALUE)->build(_args)));
    }
}

PipelineBuildingContext::VertexAttribute::VertexAttribute(String name, String type, const uint32_t divisor)
    : _name(std::move(name)), _type(std::move(type)), _divisor(divisor), _usage(RenderUtil::toAttributeLayoutType(_name, _type))
{
}

bool PipelineBuildingContext::VertexAttribute::operator<(const VertexAttribute& other) const
{
    const int32_t v1 = _usage == Attribute::USAGE_CUSTOM ? 1 : 0;
    const int32_t v2 = other._usage == Attribute::USAGE_CUSTOM ? 1 : 0;
    return v1 < v2;
}

template<> PipelineBuildingContext::LayoutBindingType StringConvert::eval<PipelineBuildingContext::LayoutBindingType>(const String& repr)
{
    constexpr enums::LookupTable<PipelineBuildingContext::LayoutBindingType, 5> table = {{
        {"image", PipelineBuildingContext::LAYOUT_BINDING_TYPE_IMAGE},
        {"sampler", PipelineBuildingContext::LAYOUT_BINDING_TYPE_SAMPLER},
        {"ssbo", PipelineBuildingContext::LAYOUT_BINDING_TYPE_SSBO},
        {"ubo", PipelineBuildingContext::LAYOUT_BINDING_TYPE_UBO},
        {"auto", PipelineBuildingContext::LAYOUT_BINDING_TYPE_AUTO}
    }};
    return enums::lookup(table, repr);
}

}
