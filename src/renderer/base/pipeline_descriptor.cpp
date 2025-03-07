#include "renderer/base/pipeline_descriptor.h"

#include "core/util/string_convert.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/pipeline_configuration.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/impl/snippet/snippet_composite.h"
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

}

PipelineDescriptor::PipelineDescriptor(Camera camera, sp<PipelineBuildingContext> buildingContext, sp<Snippet> snippet, Parameters parameters)
    : _configuration(sp<PipelineConfiguration>::make(std::move(camera), std::move(buildingContext), std::move(snippet))), _parameters(std::move(parameters)), _layout(_configuration->pipelineLayout()), _attributes(_layout),
      _samplers(_configuration->makeBindingSamplers()), _images(_configuration->makeBindingImages())
{
}

const Optional<Rect>& PipelineDescriptor::scissor() const
{
    return _parameters._scissor;
}

const sp<Snippet>& PipelineDescriptor::snippet() const
{
    return _configuration->snippet();
}

const PipelineDescriptor::Parameters& PipelineDescriptor::parameters() const
{
    return _parameters;
}

void PipelineDescriptor::setParameters(Parameters parameters)
{
    _parameters = std::move(parameters);;
}

const Camera& PipelineDescriptor::camera() const
{
    return _configuration->_camera;
}

Camera& PipelineDescriptor::camera()
{
    return _configuration->_camera;
}

const sp<PipelineLayout>& PipelineDescriptor::layout() const
{
    return _layout;
}

const PipelineLayout::AttributeOffsets& PipelineDescriptor::attributes() const
{
    return _attributes;
}

const Vector<std::pair<sp<Texture>, PipelineLayout::DescriptorSet>>& PipelineDescriptor::samplers() const
{
    return _samplers;
}

const Vector<std::pair<sp<Texture>, PipelineLayout::DescriptorSet>>& PipelineDescriptor::images() const
{
    return _images;
}

void PipelineDescriptor::preCompile(GraphicsContext& graphicsContext)
{
    _configuration->preCompile(graphicsContext, *this);
}

Map<Enum::ShaderStageBit, ShaderPreprocessor::Stage> PipelineDescriptor::getPreprocessedStages(const RenderEngineContext& renderEngineContext) const
{
    return _configuration->getPreprocessedStages(renderEngineContext);
}

void PipelineDescriptor::bindSampler(sp<Texture> texture, const uint32_t name)
{
    CHECK_WARN(_samplers.size() > name, "Illegal sampler binding position: %d, sampler count: %d", name, _samplers.size());
    if(_samplers.size() > name)
        _samplers[name].first = std::move(texture);
}

bool PipelineDescriptor::hasDivisors() const
{
    return _layout->streamLayouts().size() > 1;
}

bool PipelineDescriptor::hasTrait(const TraitType traitType) const
{
    return _parameters._traits.has(traitType);
}

PipelineDescriptor::Parameters::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _scissor(factory.getBuilder<Vec4>(manifest, "scissor"))
{
    for(const document& i : manifest->children("trait"))
        _traits.push_back(Documents::ensureAttribute<TraitType>(i, constants::TYPE), toPipelineTraitMeta(i));
}

PipelineDescriptor::Parameters PipelineDescriptor::Parameters::BUILDER::build(const Scope& args) const
{
    const sp<Vec4> scissor = _scissor.build(args);
    return {scissor ? Optional<Rect>(Ark::instance().renderController()->renderEngine()->toRendererRect(Rect(scissor->val()))) : Optional<Rect>(), _traits};
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
