#include "renderer/base/pipeline_descriptor.h"

#include "core/util/string_convert.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/impl/snippet/snippet_composite.h"
#include "renderer/impl/draw_decorator/draw_decorator_factory_compute.h"
#include "renderer/impl/draw_decorator/draw_decorator_factory_compound.h"
#include "renderer/inf/snippet_factory.h"

namespace ark {

namespace {

PipelineDescriptor::TraitStencilTestSeparate loadStencilTestSeparate(const document& manifest, const bool allowDefaultFace)
{
    PipelineDescriptor::TraitStencilTestSeparate face;
    face._type = Documents::getAttribute<PipelineDescriptor::FrontFaceType>(manifest, "face-type", PipelineDescriptor::FRONT_FACE_TYPE_DEFAULT);
    if(!allowDefaultFace && face._type == PipelineDescriptor::FRONT_FACE_TYPE_DEFAULT)
        return face;
    face._func = Documents::ensureAttribute<PipelineDescriptor::CompareFunc>(manifest, "func");
    face._mask = Documents::getAttribute<uint32_t>(manifest, "mask", 0xff);
    face._compare_mask = Documents::getAttribute<uint32_t>(manifest, "compare-mask", 0xff);
    face._ref = Documents::ensureAttribute<uint32_t>(manifest, "ref");
    face._op = Documents::getAttribute<PipelineDescriptor::StencilFunc>(manifest, "op", PipelineDescriptor::STENCIL_FUNC_KEEP);
    face._op_dfail = Documents::getAttribute<PipelineDescriptor::StencilFunc>(manifest, "op-dfail", PipelineDescriptor::STENCIL_FUNC_KEEP);
    face._op_dpass = Documents::getAttribute<PipelineDescriptor::StencilFunc>(manifest, "op-dpass", PipelineDescriptor::STENCIL_FUNC_KEEP);
    return face;
}

PipelineDescriptor::Trait toPipelineTrait(const PipelineDescriptor::TraitType traitType, const document& manifest)
{
    switch(traitType)
    {
        case PipelineDescriptor::TRAIT_TYPE_DEPTH_TEST:
        {
            const bool enabled = Documents::getAttribute<bool>(manifest, "enabled", true);
            const PipelineDescriptor::TraitDepthTest depthTest = {
                enabled,
            Documents::getAttribute<bool>(manifest, "write-enabled", enabled),
            Documents::getAttribute<PipelineDescriptor::CompareFunc>(manifest, "func", PipelineDescriptor::COMPARE_FUNC_DEFAULT)
            };
            return {depthTest};
        }
        case PipelineDescriptor::TRAIT_TYPE_STENCIL_TEST: {
            PipelineDescriptor::TraitStencilTest stencilTest;
            if(const Vector<document>& faces = manifest->children("face"); faces.empty())
                stencilTest._front = stencilTest._back = loadStencilTestSeparate(manifest, true);
            else
            {
                stencilTest._front._type = stencilTest._back._type = PipelineDescriptor::FRONT_FACE_TYPE_DEFAULT;
                for(const auto& i : faces)
                {
                    const PipelineDescriptor::TraitStencilTestSeparate face = loadStencilTestSeparate(i, false);
                    DCHECK(face._type != PipelineDescriptor::FRONT_FACE_TYPE_DEFAULT, "Default face type is not allowed in separated stencil configs");
                    (face._type == PipelineDescriptor::FRONT_FACE_TYPE_FRONT ? stencilTest._front : stencilTest._back) = face;
                }
            }
            return {stencilTest};
        }
        case PipelineDescriptor::TRAIT_TYPE_CULL_FACE_TEST:
        {
            const PipelineDescriptor::TraitCullFaceTest cullFaceTest = {
                Documents::getAttribute<bool>(manifest, "enabled", true),
                Documents::getAttribute<PipelineDescriptor::FrontFace>(manifest, "front-face", PipelineDescriptor::FRONT_FACE_DEFAULT)
            };
            return {cullFaceTest};
        }
        case PipelineDescriptor::TRAIT_TYPE_BLEND:
        {
            const PipelineDescriptor::TraitBlend blend = {
                Documents::getAttribute<bool>(manifest, "enabled", true),
                Documents::getAttribute<PipelineDescriptor::BlendFactor>(manifest, "src-rgb", PipelineDescriptor::BLEND_FACTOR_DEFAULT),
                Documents::getAttribute<PipelineDescriptor::BlendFactor>(manifest, "dst-rgb", PipelineDescriptor::BLEND_FACTOR_DEFAULT),
                Documents::getAttribute<PipelineDescriptor::BlendFactor>(manifest, "src-alpha", PipelineDescriptor::BLEND_FACTOR_DEFAULT),
                Documents::getAttribute<PipelineDescriptor::BlendFactor>(manifest, "dst-alpha", PipelineDescriptor::BLEND_FACTOR_DEFAULT)
            };
            return {blend};
        }
        case PipelineDescriptor::TRAIT_TYPE_SCISSOR_TEST:
        {
            PipelineDescriptor::TraitScissorTest scissorTest = {
                Documents::getAttribute<bool>(manifest, "dynamic", true)
            };
            return {std::move(scissorTest)};
        }
    }
    return {};
}

String preprocess(const RenderEngineContext& renderEngineContext, const Map<String, String>& definitions, const String& source)
{
    DCHECK(renderEngineContext.version() > 0, "Unintialized RenderEngineContext");

    static std::regex var_pattern(R"(\$\{([\w.]+)\})");
    const Map<String, String>& engineDefinitions = renderEngineContext.definitions();

    return source.replace(var_pattern, [&engineDefinitions, &definitions] (Array<String>& matches)->String {
        const String& varName = matches.at(1);
        auto iter = engineDefinitions.find(varName);
        if(iter != engineDefinitions.end())
            return iter->second;
        iter = definitions.find(varName);
        CHECK(iter != definitions.end(), "Undefinition \"%s\"", varName.c_str());
        return iter->second;
    });
}

sp<Snippet> createCoreSnippet(sp<Snippet> next)
{
    sp<Snippet> coreSnippet = Ark::instance().renderController()->renderEngine()->context()->snippetFactory()->createCoreSnippet();
    DASSERT(coreSnippet);
    if(next)
        return sp<Snippet>::make<SnippetComposite>(std::move(coreSnippet), std::move(next));
    return coreSnippet;
}

PipelineDescriptor::Configuration initConfiguration(PipelineDescriptor::Configuration configuration, PipelineBuildingContext& buildingContext, const Camera& camera)
{
    configuration._snippet = createCoreSnippet(std::move(configuration._snippet));
    configuration._snippet->preInitialize(buildingContext);
    buildingContext.initialize(camera);
    configuration._snippet->postInitialize(buildingContext);
    return configuration;
}

}

PipelineDescriptor::PipelineDescriptor(Camera camera, PipelineBuildingContext& buildingContext, Configuration configuration)
    : _camera(std::move(camera)), _configuration(initConfiguration(std::move(configuration), buildingContext, _camera)), _layout(buildingContext._pipeline_layout),
      _predefined_samplers(std::move(buildingContext._samplers)), _predefined_images(std::move(buildingContext._images)), _definitions(buildingContext.toDefinitions())
{
    if(const op<ShaderPreprocessor>& computeStage = buildingContext.computingStage())
    {
        V3i numWorkGroupsArray;
        if(_configuration._num_work_groups)
        {
            numWorkGroupsArray = _configuration._num_work_groups->val();
        }
        else
        {
            CHECK(computeStage->_compute_local_sizes, "Compute stage local size layout undefined");
            numWorkGroupsArray = computeStage->_compute_local_sizes.value();
        }
        _configuration._draw_decorator_factory = DrawDecoratorFactoryCompound::compound(std::move(_configuration._draw_decorator_factory), sp<DrawDecoratorFactory>::make<DrawDecoratorFactoryCompute>(_layout, numWorkGroupsArray, computeStage->_pre_shader_stage != enums::SHADER_STAGE_BIT_NONE));
    }

    for(const ShaderPreprocessor* preprocessor : buildingContext.stages())
        _stages.push_back(preprocessor->preprocess());

    _layout->initializeSSBO(buildingContext);
}

const sp<Vec4>& PipelineDescriptor::scissor() const
{
    return _configuration._scissor;
}

const sp<Snippet>& PipelineDescriptor::snippet() const
{
    return _configuration._snippet;
}

const PipelineDescriptor::Configuration& PipelineDescriptor::configuration() const
{
    return _configuration;
}

void PipelineDescriptor::setConfiguration(Configuration configuration)
{
    _configuration = std::move(configuration);;
}

const Camera& PipelineDescriptor::camera() const
{
    return _camera;
}

Camera& PipelineDescriptor::camera()
{
    return _camera;
}

const sp<PipelineLayout>& PipelineDescriptor::layout() const
{
    return _layout;
}

const PipelineLayout::VertexDescriptor& PipelineDescriptor::vertexDescriptor() const
{
    return _layout->vertexDescriptor();
}

bool PipelineDescriptor::hasDivisors() const
{
    return _layout->streamLayouts().size() > 1;
}

String PipelineDescriptor::generateSignature() const
{
    StringBuffer sb;
    for(const ShaderPreprocessor::Stage& i : _stages)
    {
        if(!sb.empty())
            sb << ", ";
        sb << Documents::toString(i._manifest);
    }
    return sb.str();
}

Map<enums::ShaderStageBit, ShaderPreprocessor::Stage> PipelineDescriptor::getPreprocessedStages(const RenderEngineContext& renderEngineContext) const
{
    Map<enums::ShaderStageBit, ShaderPreprocessor::Stage> shaders;

    for(const auto& [manifest, stage, source] : _stages)
        shaders[stage] = {manifest, stage, preprocess(renderEngineContext, _definitions, source)};

    return shaders;
}

Vector<PipelineDescriptor::BindedTexture> PipelineDescriptor::makeBindingSamplers() const
{
    const PipelineLayout& pipelineLayout = _layout;
    CHECK_WARN(pipelineLayout._samplers.size() >= _predefined_samplers.size(), "Predefined samplers(%d) is more than samplers(%d) in PipelineLayout", _predefined_samplers.size(), pipelineLayout._samplers.size());

    Vector<BindedTexture> samplers;
    for(size_t i = 0; i < pipelineLayout._samplers.size(); ++i)
    {
        const String& name = pipelineLayout._samplers.keys().at(i);
        const auto iter = _predefined_samplers.find(name);
        sp<Texture> texture = iter != _predefined_samplers.end() ? iter->second : (i < _predefined_samplers.size() ? _predefined_samplers.values().at(i) : nullptr);
        samplers.emplace_back(name, std::move(texture), pipelineLayout._samplers.values().at(i));
    }
    return samplers;
}

Vector<PipelineDescriptor::BindedTexture> PipelineDescriptor::makeBindingImages() const
{
    const PipelineLayout& pipelineLayout = _layout;
    DASSERT(_predefined_images.size() == pipelineLayout._images.size());

    Vector<BindedTexture> bindingImages;
    for(size_t i = 0; i < pipelineLayout._images.size(); ++i)
        bindingImages.emplace_back(_predefined_images.keys().at(i), _predefined_images.values().at(i), pipelineLayout._images.values().at(i));
    return bindingImages;
}

PipelineDescriptor::Configuration::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _num_work_groups(factory.getBuilder<Vec3i>(manifest, "num-work-groups"))
{
    for(const document& i : manifest->children("trait"))
    {
        const TraitType traitType = Documents::ensureAttribute<TraitType>(i, constants::TYPE);
        _traits.push_back(toPipelineTrait(traitType, i));
        if(traitType == TRAIT_TYPE_SCISSOR_TEST)
            _scissor = factory.getBuilder<Vec4>(i, "scissor");
    }
}

PipelineDescriptor::Configuration PipelineDescriptor::Configuration::BUILDER::build(const Scope& args) const
{
    return {_traits, _scissor.build(args), _num_work_groups.build(args)};
}

template<> ARK_API PipelineDescriptor::TraitType StringConvert::eval<PipelineDescriptor::TraitType>(const String& str)
{
    constexpr enums::LookupTable<PipelineDescriptor::TraitType, 5> table = {{
        {"cull_face_test", PipelineDescriptor::TRAIT_TYPE_CULL_FACE_TEST},
        {"depth_test", PipelineDescriptor::TRAIT_TYPE_DEPTH_TEST},
        {"scissor_test", PipelineDescriptor::TRAIT_TYPE_SCISSOR_TEST},
        {"stencil_test", PipelineDescriptor::TRAIT_TYPE_STENCIL_TEST},
        {"blend", PipelineDescriptor::TRAIT_TYPE_BLEND}
    }};
    return enums::lookup(table, str);
}

template<> ARK_API PipelineDescriptor::CompareFunc StringConvert::eval<PipelineDescriptor::CompareFunc>(const String& str)
{
    constexpr enums::LookupTable<PipelineDescriptor::CompareFunc, 8> table = {{
        {"always", PipelineDescriptor::COMPARE_FUNC_ALWAYS},
        {"never", PipelineDescriptor::COMPARE_FUNC_NEVER},
        {"equal", PipelineDescriptor::COMPARE_FUNC_EQUAL},
        {"not_equal", PipelineDescriptor::COMPARE_FUNC_NOT_EQUAL},
        {"less", PipelineDescriptor::COMPARE_FUNC_LESS},
        {"greater", PipelineDescriptor::COMPARE_FUNC_GREATER},
        {"less_equal", PipelineDescriptor::COMPARE_FUNC_LEQUAL},
        {"greater_equal", PipelineDescriptor::COMPARE_FUNC_GEQUAL}
    }};
    return enums::lookup(table, str);
}

template<> ARK_API PipelineDescriptor::StencilFunc StringConvert::eval<PipelineDescriptor::StencilFunc>(const String& str)
{
    constexpr enums::LookupTable<PipelineDescriptor::StencilFunc, 8> table = {{
        {"keep", PipelineDescriptor::STENCIL_FUNC_KEEP},
        {"zero", PipelineDescriptor::STENCIL_FUNC_ZERO},
        {"replace", PipelineDescriptor::STENCIL_FUNC_REPLACE},
        {"increase", PipelineDescriptor::STENCIL_FUNC_INCREASE},
        {"increase_and_wrap", PipelineDescriptor::STENCIL_FUNC_INCREASE_AND_WRAP},
        {"decrease", PipelineDescriptor::STENCIL_FUNC_DECREASE},
        {"decrease_and_wrap", PipelineDescriptor::STENCIL_FUNC_DECREASE_AND_WRAP},
        {"invert", PipelineDescriptor::STENCIL_FUNC_INVERT}
    }};
    return enums::lookup(table, str);
}

template<> ARK_API PipelineDescriptor::FrontFace StringConvert::eval<PipelineDescriptor::FrontFace>(const String& str)
{
    if(str == "ccw")
        return PipelineDescriptor::FRONT_FACE_COUTER_CLOCK_WISE;
    DCHECK(str == "cw", "Unknown FrontFace: \"%s\", possible values are [ccw, cw]", str.c_str());
    return PipelineDescriptor::FRONT_FACE_CLOCK_WISE;
}

template<> ARK_API PipelineDescriptor::FrontFaceType StringConvert::eval<PipelineDescriptor::FrontFaceType>(const String& str)
{
    constexpr enums::LookupTable<PipelineDescriptor::FrontFaceType, 2> table = {{
        {"front", PipelineDescriptor::FRONT_FACE_TYPE_FRONT},
        {"back", PipelineDescriptor::FRONT_FACE_TYPE_BACK}
    }};
    return enums::lookup(table, str);
}

template<> ARK_API PipelineDescriptor::BlendFactor StringConvert::eval<PipelineDescriptor::BlendFactor>(const String& str)
{
    constexpr enums::LookupTable<PipelineDescriptor::BlendFactor, 13> table = {{
        {"default", PipelineDescriptor::BLEND_FACTOR_DEFAULT},
        {"zero", PipelineDescriptor::BLEND_FACTOR_ZERO},
        {"one", PipelineDescriptor::BLEND_FACTOR_ONE},
        {"src_color", PipelineDescriptor::BLEND_FACTOR_SRC_COLOR},
        {"one_minus_src_color", PipelineDescriptor::BLEND_FACTOR_ONE_MINUS_SRC_COLOR},
        {"dst_color", PipelineDescriptor::BLEND_FACTOR_DST_COLOR},
        {"one_minus_dst_color", PipelineDescriptor::BLEND_FACTOR_ONE_MINUS_DST_COLOR},
        {"src_alpha", PipelineDescriptor::BLEND_FACTOR_SRC_ALPHA},
        {"one_minus_src_alpha", PipelineDescriptor::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA},
        {"dst_alpha", PipelineDescriptor::BLEND_FACTOR_DST_ALPHA},
        {"one_minus_dst_alpha", PipelineDescriptor::BLEND_FACTOR_ONE_MINUS_DST_ALPHA},
        {"const_color", PipelineDescriptor::BLEND_FACTOR_CONST_COLOR},
        {"const_alpha", PipelineDescriptor::BLEND_FACTOR_CONST_ALPHA}
    }};
    return enums::lookup(table, str);
}

}
