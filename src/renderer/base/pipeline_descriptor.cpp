#include "renderer/base/pipeline_descriptor.h"

#include "core/util/string_convert.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/impl/snippet/snippet_composite.h"
#include "renderer/impl/snippet/snippet_draw_compute.h"
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

PipelineDescriptor::TraitConfigure toPipelineTraitMeta(const document& manifest)
{
    PipelineDescriptor::TraitConfigure pipelineTrait;
    switch(Documents::ensureAttribute<PipelineDescriptor::TraitType>(manifest, constants::TYPE))
    {
        case PipelineDescriptor::TRAIT_TYPE_DEPTH_TEST:
            pipelineTrait._depth_test._enabled = Documents::getAttribute<bool>(manifest, "enabled", true);
            pipelineTrait._depth_test._write_enabled = Documents::getAttribute<bool>(manifest, "write-enabled", pipelineTrait._depth_test._enabled);
            pipelineTrait._depth_test._func = Documents::getAttribute<PipelineDescriptor::CompareFunc>(manifest, "func", PipelineDescriptor::COMPARE_FUNC_DEFAULT);
            break;
        case PipelineDescriptor::TRAIT_TYPE_STENCIL_TEST: {
            PipelineDescriptor::TraitStencilTest& stencilTest = pipelineTrait._stencil_test;
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
            break;
        }
        case PipelineDescriptor::TRAIT_TYPE_CULL_FACE_TEST:
            pipelineTrait._cull_face_test._enabled = Documents::getAttribute<bool>(manifest, "enabled", true);
            pipelineTrait._cull_face_test._front_face = Documents::getAttribute<PipelineDescriptor::FrontFace>(manifest, "front-face", PipelineDescriptor::FRONT_FACE_DEFAULT);
            break;
        case PipelineDescriptor::TRAIT_TYPE_BLEND:
            pipelineTrait._blend._enabled = Documents::getAttribute<bool>(manifest, "enabled", true);
            pipelineTrait._blend._src_rgb_factor = Documents::getAttribute<PipelineDescriptor::BlendFactor>(manifest, "src-rgb", PipelineDescriptor::BLEND_FACTOR_DEFAULT);
            pipelineTrait._blend._dst_rgb_factor = Documents::getAttribute<PipelineDescriptor::BlendFactor>(manifest, "dst-rgb", PipelineDescriptor::BLEND_FACTOR_DEFAULT);
            pipelineTrait._blend._src_alpha_factor = Documents::getAttribute<PipelineDescriptor::BlendFactor>(manifest, "src-alpha", PipelineDescriptor::BLEND_FACTOR_DEFAULT);
            pipelineTrait._blend._dst_alpha_factor = Documents::getAttribute<PipelineDescriptor::BlendFactor>(manifest, "dst-alpha", PipelineDescriptor::BLEND_FACTOR_DEFAULT);
            break;
        case PipelineDescriptor::TRAIT_TYPE_SCISSOR_TEST:
            break;
    }
    return pipelineTrait;
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

class ComputeSnippetWrapper final : public Snippet, public Wrapper<Snippet> {
public:
    ComputeSnippetWrapper()
        : Wrapper() {
    }

    sp<DrawDecorator> makeDrawDecorator(const RenderRequest& renderRequest) override
    {
        return _wrapped->makeDrawDecorator(renderRequest);
    }
};


sp<Snippet> createSnippet(const Camera& camera, sp<Snippet> snippet, PipelineBuildingContext& buildingContext)
{
    sp<ComputeSnippetWrapper> computeSnippetWrapper;
    if(const op<ShaderPreprocessor>& computeStage = buildingContext.computingStage(); computeStage && !buildingContext.renderStages().empty())
    {
        computeSnippetWrapper = sp<ComputeSnippetWrapper>::make();
        snippet = SnippetComposite::compose(std::move(snippet), std::move(snippet));
    }

    snippet = createCoreSnippet(std::move(snippet));
    snippet->preInitialize(buildingContext);
    buildingContext.initialize(camera);

    if(computeSnippetWrapper)
    {
        std::array<uint32_t, 3> numWorkGroupsArray = {1, 1, 1};
        const op<ShaderPreprocessor>& computeStage = buildingContext.computingStage();
        if(const String numWorkGroupsAttr = Documents::getAttribute(computeStage->_manifest, "num-work-groups"))
        {
            const Vector<String> numWorkGroups = numWorkGroupsAttr.split(',');
            for(size_t i = 0; i < std::min(numWorkGroups.size(), numWorkGroupsArray.size()); ++i)
                numWorkGroupsArray[i] = Strings::eval<uint32_t>(numWorkGroups.at(i));
        }
        else
        {
            CHECK(computeStage->_compute_local_sizes, "Compute stage local size layout undefined");
            numWorkGroupsArray = computeStage->_compute_local_sizes.value();
        }
        computeSnippetWrapper->reset(sp<Snippet>::make<SnippetDrawCompute>(buildingContext._pipeline_layout, numWorkGroupsArray, computeStage->_pre_shader_stage != Enum::SHADER_STAGE_BIT_NONE));
    }

    return snippet;
}

}

PipelineDescriptor::PipelineDescriptor(Camera camera, sp<PipelineBuildingContext> buildingContext, Configuration configuration)
    : _camera(std::move(camera)), _configuration(std::move(configuration)), _building_context(std::move(buildingContext)), _layout(_building_context->_pipeline_layout),
      _predefined_samplers(std::move(_building_context->_samplers)), _predefined_images(std::move(_building_context->_images)), _definitions(_building_context->toDefinitions())
{
    _configuration._snippet = createSnippet(_camera, std::move(_configuration._snippet), _building_context);
}

const sp<Vec4>& PipelineDescriptor::scissor() const
{
    return _configuration._scissor;
}

const sp<Snippet>& PipelineDescriptor::snippet() const
{
    return _configuration._snippet;
}

const PipelineDescriptor::Configuration& PipelineDescriptor::parameters() const
{
    return _configuration;
}

void PipelineDescriptor::setParameters(Configuration parameters)
{
    _configuration = std::move(parameters);;
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

bool PipelineDescriptor::hasTrait(const TraitType traitType) const
{
    return _configuration._traits.has(traitType);
}

void PipelineDescriptor::preCompile(GraphicsContext& graphicsContext)
{
    if(_building_context)
    {
        _configuration._snippet->preCompile(graphicsContext, _building_context, *this);

        for(const ShaderPreprocessor* preprocessor : _building_context->stages())
            _stages.push_back(preprocessor->preprocess());

        _building_context = nullptr;
    }
}

Map<Enum::ShaderStageBit, ShaderPreprocessor::Stage> PipelineDescriptor::getPreprocessedStages(const RenderEngineContext& renderEngineContext) const
{
    Map<Enum::ShaderStageBit, ShaderPreprocessor::Stage> shaders;

    for(const auto& [manifest, stage, source] : _stages)
        shaders[stage] = {manifest, stage, preprocess(renderEngineContext, _definitions, source)};

    return shaders;
}

Vector<std::pair<sp<Texture>, PipelineLayout::DescriptorSet>> PipelineDescriptor::makeBindingSamplers() const
{
    const PipelineLayout& pipelineLayout = _layout;
    CHECK_WARN(pipelineLayout._samplers.size() >= _predefined_samplers.size(), "Predefined samplers(%d) is more than samplers(%d) in PipelineLayout", _predefined_samplers.size(), pipelineLayout._samplers.size());

    Vector<std::pair<sp<Texture>, PipelineLayout::DescriptorSet>> samplers;
    for(size_t i = 0; i < pipelineLayout._samplers.size(); ++i)
    {
        const String& name = pipelineLayout._samplers.keys().at(i);
        const auto iter = _predefined_samplers.find(name);
        samplers.emplace_back(iter != _predefined_samplers.end() ? iter->second : (i < _predefined_samplers.size() ? _predefined_samplers.values().at(i) : nullptr), pipelineLayout._samplers.values().at(i));
    }
    return samplers;
}

Vector<std::pair<sp<Texture>, PipelineLayout::DescriptorSet>> PipelineDescriptor::makeBindingImages() const
{
    const PipelineLayout& pipelineLayout = _layout;
    DASSERT(_predefined_images.size() == pipelineLayout._images.size());

    Vector<std::pair<sp<Texture>, PipelineLayout::DescriptorSet>> bindingImages;
    for(size_t i = 0; i < pipelineLayout._images.size(); ++i)
        bindingImages.emplace_back(_predefined_images.values().at(i), pipelineLayout._images.values().at(i));
    return bindingImages;
}

PipelineDescriptor::Configuration::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _scissor(factory.getBuilder<Vec4>(manifest, "scissor"))
{
    for(const document& i : manifest->children("trait"))
        _traits.push_back(Documents::ensureAttribute<TraitType>(i, constants::TYPE), toPipelineTraitMeta(i));
}

PipelineDescriptor::Configuration PipelineDescriptor::Configuration::BUILDER::build(const Scope& args) const
{
    return {_traits, _scissor.build(args)};
}

template<> ARK_API PipelineDescriptor::TraitType StringConvert::eval<PipelineDescriptor::TraitType>(const String& str)
{
    constexpr Enum::LookupTable<StringView, PipelineDescriptor::TraitType, 5> table = {{
        {"cull_face_test", PipelineDescriptor::TRAIT_TYPE_CULL_FACE_TEST},
        {"depth_test", PipelineDescriptor::TRAIT_TYPE_DEPTH_TEST},
        {"scissor_test", PipelineDescriptor::TRAIT_TYPE_SCISSOR_TEST},
        {"stencil_test", PipelineDescriptor::TRAIT_TYPE_STENCIL_TEST},
        {"blend", PipelineDescriptor::TRAIT_TYPE_BLEND}
    }};
    return Enum::lookup<PipelineDescriptor::TraitType, 5>(table, str);
}

template<> ARK_API PipelineDescriptor::CompareFunc StringConvert::eval<PipelineDescriptor::CompareFunc>(const String& str)
{
    constexpr Enum::LookupTable<StringView, PipelineDescriptor::CompareFunc, 8> table = {{
        {"always", PipelineDescriptor::COMPARE_FUNC_ALWAYS},
        {"never", PipelineDescriptor::COMPARE_FUNC_NEVER},
        {"equal", PipelineDescriptor::COMPARE_FUNC_EQUAL},
        {"not_equal", PipelineDescriptor::COMPARE_FUNC_NOT_EQUAL},
        {"less", PipelineDescriptor::COMPARE_FUNC_LESS},
        {"greater", PipelineDescriptor::COMPARE_FUNC_GREATER},
        {"less_equal", PipelineDescriptor::COMPARE_FUNC_LEQUAL},
        {"greater_equal", PipelineDescriptor::COMPARE_FUNC_GEQUAL}
    }};
    return Enum::lookup<PipelineDescriptor::CompareFunc, 8>(table, str);
}

template<> ARK_API PipelineDescriptor::StencilFunc StringConvert::eval<PipelineDescriptor::StencilFunc>(const String& str)
{
    constexpr Enum::LookupTable<StringView, PipelineDescriptor::StencilFunc, 8> table = {{
        {"keep", PipelineDescriptor::STENCIL_FUNC_KEEP},
        {"zero", PipelineDescriptor::STENCIL_FUNC_ZERO},
        {"replace", PipelineDescriptor::STENCIL_FUNC_REPLACE},
        {"increase", PipelineDescriptor::STENCIL_FUNC_INCREASE},
        {"increase_and_wrap", PipelineDescriptor::STENCIL_FUNC_INCREASE_AND_WRAP},
        {"decrease", PipelineDescriptor::STENCIL_FUNC_DECREASE},
        {"decrease_and_wrap", PipelineDescriptor::STENCIL_FUNC_DECREASE_AND_WRAP},
        {"invert", PipelineDescriptor::STENCIL_FUNC_INVERT}
    }};
    return Enum::lookup<PipelineDescriptor::StencilFunc, 8>(table, str);
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
    constexpr Enum::LookupTable<StringView, PipelineDescriptor::FrontFaceType, 2> table = {{
        {"front", PipelineDescriptor::FRONT_FACE_TYPE_FRONT},
        {"back", PipelineDescriptor::FRONT_FACE_TYPE_BACK}
    }};
    return Enum::lookup<PipelineDescriptor::FrontFaceType, 2>(table, str);
}

template<> ARK_API PipelineDescriptor::BlendFactor StringConvert::eval<PipelineDescriptor::BlendFactor>(const String& str)
{
    constexpr Enum::LookupTable<StringView, PipelineDescriptor::BlendFactor, 13> table = {{
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
    return Enum::lookup<PipelineDescriptor::BlendFactor, 13>(table, str);
}

}
