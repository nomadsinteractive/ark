#include "renderer/base/pipeline_bindings.h"

#include "core/util/conversions.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/inf/pipeline.h"
#include "renderer/inf/pipeline_factory.h"
#include "renderer/inf/snippet.h"

namespace ark {

PipelineBindings::PipelineBindings(ModelLoader::RenderMode mode, RenderProcedure renderProcedure, Parameters parameters, sp<PipelineLayout> pipelineLayout)
    : _stub(sp<Stub>::make(mode, renderProcedure, std::move(parameters), std::move(pipelineLayout)))
{
}

ModelLoader::RenderMode PipelineBindings::mode() const
{
    return _stub->_mode;
}

PipelineBindings::RenderProcedure PipelineBindings::renderProcedure() const
{
    return _stub->_render_procedure;
}

const Rect& PipelineBindings::scissor() const
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

const PipelineInput::Attributes& PipelineBindings::attributes() const
{
    return _stub->_attributes;
}

const std::vector<sp<Texture>>& PipelineBindings::samplers() const
{
    return _stub->_samplers;
}

void PipelineBindings::bindSampler(const sp<Texture>& texture, uint32_t name)
{
    DWARN(_stub->_samplers.size() > name, "Illegal sampler binding position: %d, sampler count: %d", name, _stub->_samplers.size());
    if(_stub->_samplers.size() > name)
        _stub->_samplers[name] = texture;
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
            _pipeline->upload(graphicsContext, nullptr);
        return _pipeline;
    }

    layout()->preCompile(graphicsContext);
    _pipeline = pipelineFactory->buildPipeline(graphicsContext, *this);
    graphicsContext.renderController()->upload(_pipeline, nullptr, RenderController::US_ON_SURFACE_READY, RenderController::UPLOAD_PRIORITY_HIGH);
    _pipeline->upload(graphicsContext, nullptr);
    return _pipeline;
}

PipelineBindings::Stub::Stub(ModelLoader::RenderMode mode, RenderProcedure renderProcedure, Parameters parameters, sp<PipelineLayout> pipelineLayout)
    : _mode(mode), _render_procedure(renderProcedure), _parameters(std::move(parameters)), _layout(std::move(pipelineLayout)), _input(_layout->input()), _attributes(_input)
{
    _samplers.resize(_input->samplerCount());

    const Table<String, sp<Texture>>& samplers = _layout->samplers();
    DWARN(_samplers.size() >= samplers.size(), "Predefined samplers(%d) is more than samplers(%d) in PipelineLayout", samplers.size(), _samplers.size());

    for(size_t i = 0; i < samplers.values().size(); ++i)
        if(i < _samplers.size())
            _samplers[i] = samplers.values().at(i);
}

PipelineBindings::Parameters::Parameters(const Rect& scissor, PipelineBindings::FragmentTestTable tests, uint32_t flags)
    : _scissor(scissor), _tests(std::move(tests)), _flags(flags)
{
}

PipelineBindings::Parameters::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _render_controller(resourceLoaderContext->renderController()), _pipeline_bindings_scissor(factory.getBuilder<Vec4>(manifest, "scissor")), _pipeline_bindings_flags(Documents::getAttribute<PipelineBindings::Flag>(manifest, "flags", PipelineBindings::FLAG_DEFAULT_VALUE))
{
    for(const document& i : manifest->children("test"))
        _tests.push_back(Documents::ensureAttribute<FragmentTest>(i, Constants::Attributes::TYPE), i);
}

PipelineBindings::Parameters PipelineBindings::Parameters::BUILDER::build(const Scope& args) const
{
    sp<Vec4> scissor = _pipeline_bindings_scissor->build(args);
    return Parameters(_render_controller->renderEngine()->toRendererScissor(scissor ? Rect(scissor->val()) : Rect()), _tests, _pipeline_bindings_flags);
}

template<> ARK_API PipelineBindings::Flag Conversions::to<String, PipelineBindings::Flag>(const String& str)
{
    int32_t flag = 0;
    for(const String& i : str.split('|'))
        if(i == "cull_mode_none")
        {
            DCHECK((flag & PipelineBindings::FLAG_CULL_MODE_BITMASK) == 0, "Exclusive flag found in \"%s\"", str.c_str());
            flag |= static_cast<int32_t>(PipelineBindings::FLAG_CULL_MODE_NONE);
        }
        else if(i == "cull_mode_cw")
        {
            DCHECK((flag & PipelineBindings::FLAG_CULL_MODE_BITMASK) == 0, "Exclusive flag found in \"%s\"", str.c_str());
            flag |= static_cast<int32_t>(PipelineBindings::FLAG_CULL_MODE_CW);
        }
        else if(i == "cull_mode_ccw")
        {
            DCHECK((flag & PipelineBindings::FLAG_CULL_MODE_BITMASK) == 0, "Exclusive flag found in \"%s\"", str.c_str());
            flag |= static_cast<int32_t>(PipelineBindings::FLAG_CULL_MODE_CCW);
        }
        else
        {
            DCHECK(i == "dynamic_scissor", "Unknow PipelineBindings flag: %s, available values are [\"cull_mode_none\", \"cull_mode_cw\", \"cull_mode_ccw\", \"dynamic_scissor\"]", i.c_str());
            flag |= static_cast<int32_t>(PipelineBindings::FLAG_DYNAMIC_SCISSOR);
        }
    return static_cast<PipelineBindings::Flag>(flag);
}

PipelineBindings::FragmentTestManifest::FragmentTestManifest(const document& manifest)
    : _type(Documents::ensureAttribute<FragmentTest>(manifest, Constants::Attributes::TYPE))
{
    if(_type == FRAGMENT_TEST_DEPTH)
    {
        _trait._depth_test._enabled = Documents::getAttribute<bool>(manifest, "enabled", true);
        _trait._depth_test._write_enabled = Documents::getAttribute<bool>(manifest, "write-enabled", true);
        _trait._depth_test._func = Documents::getAttribute<CompareFunc>(manifest, "func", PipelineBindings::COMPARE_FUNC_DEFAULT);
    }
    else if(_type == FRAGMENT_TEST_STENCIL)
    {
        TraitStencilTest& stencilTest = _trait._stencil_test;
        const std::vector<document>& faces = manifest->children("face");
        if(faces.size() == 0)
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
    }
    else if(_type == FRAGMENT_TEST_CULL_FACE)
    {
        _trait._cull_face_test._enabled = Documents::getAttribute<bool>(manifest, "enabled", true);
        _trait._cull_face_test._front_face = Documents::getAttribute<FrontFace>(manifest, "front-face", PipelineBindings::FRONT_FACE_DEFAULT);
    }
}

PipelineBindings::TraitStencilTestSeparate PipelineBindings::FragmentTestManifest::loadStencilTestSeparate(const document& manifest, bool allowDefaultFace) const
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

template<> ARK_API PipelineBindings::FragmentTest Conversions::to<String, PipelineBindings::FragmentTest>(const String& str)
{
    if(str == "cull_face")
        return PipelineBindings::FRAGMENT_TEST_CULL_FACE;
    if(str == "depth")
        return PipelineBindings::FRAGMENT_TEST_DEPTH;
    else if(str == "scissor")
        return PipelineBindings::FRAGMENT_TEST_SCISSOR;
    DCHECK(str == "stencil", "Unknown FragmentTest: \"%s\", possible values are [cull_face, depth, scissor, stencil]", str.c_str());
    return PipelineBindings::FRAGMENT_TEST_STENCIL;
}

template<> ARK_API PipelineBindings::CompareFunc Conversions::to<String, PipelineBindings::CompareFunc>(const String& str)
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
    DCHECK(str == "greater_equal", "Unknown CompareFunc: \"%s\", possible values are [always, never, equal, not_equal, less, greater, less_equal, greater_equal]", str.c_str());
    return PipelineBindings::COMPARE_FUNC_GEQUAL;
}

template<> ARK_API PipelineBindings::StencilFunc Conversions::to<String, PipelineBindings::StencilFunc>(const String& str)
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

template<> ARK_API PipelineBindings::FrontFace Conversions::to<String, PipelineBindings::FrontFace>(const String& str)
{
    if(str == "ccw")
        return PipelineBindings::FRONT_FACE_COUTER_CLOCK_WISE;
    DCHECK(str == "cw", "Unknown FrontFace: \"%s\", possible values are [ccw, cw]", str.c_str());
    return PipelineBindings::FRONT_FACE_CLOCK_WISE;
}

template<> ARK_API PipelineBindings::FrontFaceType Conversions::to<String, PipelineBindings::FrontFaceType>(const String& str)
{
    if(str == "front")
        return PipelineBindings::FRONT_FACE_TYPE_FRONT;
    DCHECK(str == "back", "Unknown FrontFaceType: \"%s\", possible values are [front, back]", str.c_str());
    return PipelineBindings::FRONT_FACE_TYPE_BACK;
}

}
