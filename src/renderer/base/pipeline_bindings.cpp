#include "renderer/base/pipeline_bindings.h"

#include "core/base/enum_map.h"
#include "core/util/string_convert.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/inf/pipeline.h"
#include "renderer/inf/pipeline_factory.h"
#include "renderer/inf/snippet.h"

namespace ark {

PipelineBindings::PipelineBindings(Enum::RenderMode mode, Enum::DrawProcedure renderProcedure, Parameters parameters, sp<PipelineLayout> pipelineLayout)
    : _stub(sp<Stub>::make(mode, renderProcedure, std::move(parameters), std::move(pipelineLayout)))
{
}

Enum::RenderMode PipelineBindings::mode() const
{
    return _stub->_mode;
}

Enum::DrawProcedure PipelineBindings::drawProcedure() const
{
    return _stub->_render_procedure;
}

const Optional<Rect>& PipelineBindings::scissor() const
{
    return _stub->_parameters._scissor;
}

const PipelineBindings::Parameters& PipelineBindings::parameters() const
{
    return _stub->_parameters;
}

const sp<PipelineLayout>& PipelineBindings::layout() const
{
    return _stub->_layout;
}

const sp<PipelineInput>& PipelineBindings::input() const
{
    return _stub->_input;
}

const sp<Snippet>& PipelineBindings::snippet() const
{
    return _stub->_layout->snippet();
}

const PipelineInput::AttributeOffsets& PipelineBindings::attributes() const
{
    return _stub->_attributes;
}

const Table<String, sp<Texture>>& PipelineBindings::samplers() const
{
    return _stub->_samplers;
}

const std::vector<sp<Texture>>& PipelineBindings::images() const
{
    return _stub->_images;
}

void PipelineBindings::bindSampler(sp<Texture> texture, uint32_t name)
{
    CHECK_WARN(_stub->_samplers.size() > name, "Illegal sampler binding position: %d, sampler count: %d", name, _stub->_samplers.size());
    if(_stub->_samplers.size() > name)
        _stub->_samplers.values()[name] = std::move(texture);
}

bool PipelineBindings::hasDivisors() const
{
    return _stub->_input->streams().size() > 1;
}

PipelineBindings::Flag PipelineBindings::getFlag(PipelineBindings::Flag bitmask) const
{
    return static_cast<Flag>(_stub->_parameters._flags & static_cast<uint32_t>(bitmask));
}

bool PipelineBindings::hasFlag(PipelineBindings::Flag flag, PipelineBindings::Flag bitmask) const
{
    return flag == getFlag(bitmask);
}

void PipelineBindings::setFlag(PipelineBindings::Flag flag, PipelineBindings::Flag bitmask) const
{
    _stub->_parameters._flags = static_cast<uint32_t>(_stub->_parameters._flags & ~static_cast<uint32_t>(bitmask)) | static_cast<uint32_t>(flag);
}

sp<Pipeline> PipelineBindings::getPipeline(GraphicsContext& graphicsContext, const sp<PipelineFactory>& pipelineFactory)
{
    if(_pipeline)
    {
        if(_pipeline->id() == 0)
            _pipeline->upload(graphicsContext);
        return _pipeline;
    }

    layout()->preCompile(graphicsContext);
    _pipeline = pipelineFactory->buildPipeline(graphicsContext, *this);
    graphicsContext.renderController()->upload(_pipeline, RenderController::US_ON_SURFACE_READY, nullptr, nullptr, RenderController::UPLOAD_PRIORITY_HIGH);
    _pipeline->upload(graphicsContext);
    return _pipeline;
}

PipelineBindings::Stub::Stub(Enum::RenderMode mode, Enum::DrawProcedure renderProcedure, Parameters parameters, sp<PipelineLayout> pipelineLayout)
    : _mode(mode), _render_procedure(renderProcedure), _parameters(std::move(parameters)), _layout(std::move(pipelineLayout)), _input(_layout->input()), _attributes(_input),
      _samplers(_layout->samplers()), _images(_layout->images())
{
}

PipelineBindings::Parameters::Parameters(Optional<Rect> scissor, PipelineBindings::PipelineTraitTable tests, uint32_t flags)
    : _scissor(std::move(scissor)), _traits(std::move(tests)), _flags(flags)
{
}

PipelineBindings::Parameters::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const ResourceLoaderContext& resourceLoaderContext)
    : _render_controller(resourceLoaderContext.renderController()), _pipeline_bindings_scissor(factory.getBuilder<Vec4>(manifest, "scissor")), _pipeline_bindings_flags(Documents::getAttribute<PipelineBindings::Flag>(manifest, "flags", PipelineBindings::FLAG_DEFAULT_VALUE))
{
    for(const document& i : manifest->children("trait"))
        _traits.push_back(Documents::ensureAttribute<TraitType>(i, constants::TYPE), i);
}

PipelineBindings::Parameters PipelineBindings::Parameters::BUILDER::build(const Scope& args) const
{
    const sp<Vec4> scissor = _pipeline_bindings_scissor->build(args);
    return Parameters(scissor ? Optional<Rect>(_render_controller->renderEngine()->toRendererRect(Rect(scissor->val()))) : Optional<Rect>(), _traits, _pipeline_bindings_flags);
}

template<> ARK_API PipelineBindings::Flag StringConvert::eval<PipelineBindings::Flag>(const String& str)
{
    int32_t flag = 0;
    {
        DCHECK(str == "dynamic_scissor", "Unknow PipelineBindings flag: %s, available values are [\"dynamic_scissor\"]", str.c_str());
        flag |= static_cast<int32_t>(PipelineBindings::FLAG_DYNAMIC_SCISSOR);
    }
    return static_cast<PipelineBindings::Flag>(flag);
}

PipelineBindings::PipelineTraitMeta::PipelineTraitMeta(const document& manifest)
    : _type(Documents::ensureAttribute<TraitType>(manifest, constants::TYPE))
{
    switch(_type) {
    case TRAIT_TYPE_DEPTH_TEST:
        _configure._depth_test._enabled = Documents::getAttribute<bool>(manifest, "enabled", true);
        _configure._depth_test._write_enabled = Documents::getAttribute<bool>(manifest, "write-enabled", true);
        _configure._depth_test._func = Documents::getAttribute<CompareFunc>(manifest, "func", PipelineBindings::COMPARE_FUNC_DEFAULT);
        break;
    case TRAIT_TYPE_STENCIL_TEST: {
        TraitStencilTest& stencilTest = _configure._stencil_test;
        const std::vector<document>& faces = manifest->children("face");
        if(faces.empty())
            stencilTest._front = stencilTest._back = loadStencilTestSeparate(manifest, true);
        else
        {
            stencilTest._front._type = stencilTest._back._type = FRONT_FACE_TYPE_DEFAULT;
            for(const auto& i : faces)
            {
                PipelineBindings::TraitStencilTestSeparate face = loadStencilTestSeparate(i, false);
                DCHECK(face._type != FRONT_FACE_TYPE_DEFAULT, "Default face type is not allowed in separated stencil configs");
                (face._type == FRONT_FACE_TYPE_FRONT ? stencilTest._front : stencilTest._back) = face;
            }
        }
        break;
    }
    case TRAIT_TYPE_CULL_FACE_TEST:
        _configure._cull_face_test._enabled = Documents::getAttribute<bool>(manifest, "enabled", true);
        _configure._cull_face_test._front_face = Documents::getAttribute<FrontFace>(manifest, "front-face", PipelineBindings::FRONT_FACE_DEFAULT);
        break;
    case TRAIT_TYPE_BLEND:
        _configure._blend._src_rgb_factor = Documents::getAttribute<BlendFactor>(manifest, "src-rgb", BLEND_FACTOR_DEFAULT);
        _configure._blend._dst_rgb_factor = Documents::getAttribute<BlendFactor>(manifest, "dst-rgb", BLEND_FACTOR_DEFAULT);
        _configure._blend._src_alpha_factor = Documents::getAttribute<BlendFactor>(manifest, "src-alpha", BLEND_FACTOR_DEFAULT);
        _configure._blend._dst_alpha_factor = Documents::getAttribute<BlendFactor>(manifest, "dst-alpha", BLEND_FACTOR_DEFAULT);
        break;
    }
}

PipelineBindings::PipelineTraitMeta::PipelineTraitMeta(TraitType type, const TraitConfigure& configure)
    : _type(type), _configure(configure)
{
}

PipelineBindings::TraitStencilTestSeparate PipelineBindings::PipelineTraitMeta::loadStencilTestSeparate(const document& manifest, bool allowDefaultFace) const
{
    PipelineBindings::TraitStencilTestSeparate face;
    face._type = Documents::getAttribute<FrontFaceType>(manifest, "face-type", FRONT_FACE_TYPE_DEFAULT);
    if(!allowDefaultFace && face._type == FRONT_FACE_TYPE_DEFAULT)
        return face;
    face._func = Documents::ensureAttribute<CompareFunc>(manifest, "func");
    face._mask = Documents::getAttribute<uint32_t>(manifest, "mask", 0xff);
    face._compare_mask = Documents::getAttribute<uint32_t>(manifest, "compare-mask", 0xff);
    face._ref = Documents::ensureAttribute<uint32_t>(manifest, "ref");
    face._op = Documents::getAttribute<StencilFunc>(manifest, "op", STENCIL_FUNC_KEEP);
    face._op_dfail = Documents::getAttribute<StencilFunc>(manifest, "op-dfail", STENCIL_FUNC_KEEP);
    face._op_dpass = Documents::getAttribute<StencilFunc>(manifest, "op-dpass", STENCIL_FUNC_KEEP);
    return face;
}

template<> ARK_API PipelineBindings::TraitType StringConvert::eval<PipelineBindings::TraitType>(const String& str)
{
    if(str == "cull_face_test")
        return PipelineBindings::TRAIT_TYPE_CULL_FACE_TEST;
    if(str == "depth_test")
        return PipelineBindings::TRAIT_TYPE_DEPTH_TEST;
    if(str == "scissor")
        return PipelineBindings::TRAIT_TYPE_SCISSOR;
    if(str == "blend")
        return PipelineBindings::TRAIT_TYPE_BLEND;
    CHECK(str == "stencil", "Unknown FragmentTest: \"%s\", possible values are [cull_face_test, depth_test, scissor, stencil]", str.c_str());
    return PipelineBindings::TRAIT_TYPE_STENCIL_TEST;
}

template<> ARK_API PipelineBindings::CompareFunc StringConvert::eval<PipelineBindings::CompareFunc>(const String& str)
{
    if(str == "always")
        return PipelineBindings::COMPARE_FUNC_ALWAYS;
    if(str == "never")
        return PipelineBindings::COMPARE_FUNC_NEVER;
    if(str == "equal")
        return PipelineBindings::COMPARE_FUNC_EQUAL;
    if(str == "not_equal")
        return PipelineBindings::COMPARE_FUNC_NOT_EQUAL;
    if(str == "less")
        return PipelineBindings::COMPARE_FUNC_LESS;
    if(str == "greater")
        return PipelineBindings::COMPARE_FUNC_GREATER;
    if(str == "less_equal")
        return PipelineBindings::COMPARE_FUNC_LEQUAL;
    CHECK(str == "greater_equal", "Unknown CompareFunc: \"%s\", possible values are [always, never, equal, not_equal, less, greater, less_equal, greater_equal]", str.c_str());
    return PipelineBindings::COMPARE_FUNC_GEQUAL;
}

template<> ARK_API PipelineBindings::StencilFunc StringConvert::eval<PipelineBindings::StencilFunc>(const String& str)
{
    if(str == "keep")
        return PipelineBindings::STENCIL_FUNC_KEEP;
    if(str == "zero")
        return PipelineBindings::STENCIL_FUNC_ZERO;
    if(str == "replace")
        return PipelineBindings::STENCIL_FUNC_REPLACE;
    if(str == "increase")
        return PipelineBindings::STENCIL_FUNC_INCREASE;
    if(str == "increase_and_wrap")
        return PipelineBindings::STENCIL_FUNC_INCREASE_AND_WRAP;
    if(str == "decrease")
        return PipelineBindings::STENCIL_FUNC_DECREASE;
    if(str == "decrease_and_wrap")
        return PipelineBindings::STENCIL_FUNC_DECREASE_AND_WRAP;
    DCHECK(str == "invert", "Unknown StencilFunc: \"%s\", possible values are [keep, zero, replace, increase, increase_and_wrap, decrease, decrease_and_wrap, invert]", str.c_str());
    return PipelineBindings::STENCIL_FUNC_INVERT;
}

template<> ARK_API PipelineBindings::FrontFace StringConvert::eval<PipelineBindings::FrontFace>(const String& str)
{
    if(str == "ccw")
        return PipelineBindings::FRONT_FACE_COUTER_CLOCK_WISE;
    DCHECK(str == "cw", "Unknown FrontFace: \"%s\", possible values are [ccw, cw]", str.c_str());
    return PipelineBindings::FRONT_FACE_CLOCK_WISE;
}

template<> ARK_API PipelineBindings::FrontFaceType StringConvert::eval<PipelineBindings::FrontFaceType>(const String& str)
{
    if(str == "front")
        return PipelineBindings::FRONT_FACE_TYPE_FRONT;
    DCHECK(str == "back", "Unknown FrontFaceType: \"%s\", possible values are [front, back]", str.c_str());
    return PipelineBindings::FRONT_FACE_TYPE_BACK;
}

template<> ARK_API void EnumMap<PipelineBindings::BlendFactor>::initialize(std::map<String, PipelineBindings::BlendFactor>& enums)
{
    enums["default"] = PipelineBindings::BLEND_FACTOR_DEFAULT;
    enums["zero"] = PipelineBindings::BLEND_FACTOR_ZERO;
    enums["one"] = PipelineBindings::BLEND_FACTOR_ONE;
    enums["src_color"] = PipelineBindings::BLEND_FACTOR_SRC_COLOR;
    enums["one_minus_src_color"] = PipelineBindings::BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    enums["dst_color"] = PipelineBindings::BLEND_FACTOR_DST_COLOR;
    enums["one_minus_dst_color"] = PipelineBindings::BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    enums["src_alpha"] = PipelineBindings::BLEND_FACTOR_SRC_ALPHA;
    enums["one_minus_src_alpha"] = PipelineBindings::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    enums["dst_alpha"] = PipelineBindings::BLEND_FACTOR_DST_ALPHA;
    enums["one_minus_dst_alpha"] = PipelineBindings::BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    enums["const_color"] = PipelineBindings::BLEND_FACTOR_CONST_COLOR;
    enums["const_alpha"] = PipelineBindings::BLEND_FACTOR_CONST_ALPHA;
}

template<> ARK_API PipelineBindings::BlendFactor StringConvert::eval<PipelineBindings::BlendFactor>(const String& str)
{
    return EnumMap<PipelineBindings::BlendFactor>::instance().ensureEnum(str);
}

}
