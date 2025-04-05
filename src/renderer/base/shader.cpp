#include "renderer/base/shader.h"

#include "core/base/string_table.h"
#include "core/types/global.h"
#include "core/util/string_convert.h"
#include "core/util/documents.h"

#include "graphics/base/camera.h"
#include "renderer/base/varyings.h"

#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/impl/snippet/snippet_composite.h"

namespace ark {

namespace {

std::pair<Optional<const Shader::StageManifest&>, size_t> findStageManifest(const Enum::ShaderStageBit type, const Vector<Shader::StageManifest>& stages)
{
    for(size_t i = 0; i < stages.size(); ++ i)
        if(type == stages.at(i)._type)
            return {stages.at(i), i};
    return {{}, 0};
}

template<typename T, typename U> auto findInKeywordPairs(const T& kws, U key) -> decltype(std::declval<T>().front().second)
{
    for(const auto& [k, v] : kws)
        if(k == key)
            return v;
    return {};
}

Vector<std::pair<uint32_t, Buffer>> makeDivivedBuffers(const PipelineLayout& pipelineLayout, const Vector<std::pair<uint32_t, sp<Uploader>>>& uploaders)
{
    Vector<std::pair<uint32_t, Buffer>> dividedBuffers;
    for(const auto& [divisor, _] : pipelineLayout.streamLayouts())
        if(divisor != 0)
            dividedBuffers.emplace_back(divisor, Ark::instance().renderController()->makeVertexBuffer(Buffer::USAGE_BIT_DYNAMIC, findInKeywordPairs(uploaders, divisor)));
    return dividedBuffers;
}

}

Shader::StageManifest::StageManifest(Enum::ShaderStageBit type, builder<String> source)
    : _type(type), _source(std::move(source)), _manifest(Global<Constants>()->DOCUMENT_NONE)
{
}

Shader::StageManifest::StageManifest(BeanFactory& factory, const document& manifest)
    : _type(Documents::ensureAttribute<Enum::ShaderStageBit>(manifest, constants::TYPE)), _source(factory.ensureBuilder<String>(manifest, constants::SRC)), _manifest(manifest)
{
}

Shader::Shader(sp<PipelineDescriptor> pipelineDescriptor)
    : _pipeline_desciptor(std::move(pipelineDescriptor))
{
}

sp<Builder<Shader>> Shader::fromDocument(BeanFactory& factory, const document& manifest, const String& defVertex, const String& defFragment, const sp<Camera>& defaultCamera)
{
    if(builder<Shader> shader = factory.getBuilder<Shader>(manifest, constants::SHADER))
        return shader;
    const Global<StringTable> stringTable;
    Vector<StageManifest> stageManifests;
    stageManifests.emplace_back(Enum::SHADER_STAGE_BIT_VERTEX, builder<String>::make<Builder<String>::Prebuilt>(sp<String>::make(std::move(stringTable->getString(defVertex, true).value()))));
    stageManifests.emplace_back(Enum::SHADER_STAGE_BIT_FRAGMENT, builder<String>::make<Builder<String>::Prebuilt>(sp<String>::make(std::move(stringTable->getString(defFragment, true).value()))));
    return builder<Shader>::make<BUILDER_IMPL>(factory, manifest, defaultCamera ? builder<Camera>::make<Builder<Camera>::Prebuilt>(defaultCamera) : nullptr, std::move(stageManifests));
}

sp<RenderBufferSnapshot> Shader::takeBufferSnapshot(const RenderRequest& renderRequest, const bool isComputeStage) const
{
    return _pipeline_desciptor->layout()->takeBufferSnapshot(renderRequest, isComputeStage);
}

const Camera& Shader::camera() const
{
    return _pipeline_desciptor->camera();
}

const sp<PipelineLayout>& Shader::layout() const
{
    return _pipeline_desciptor->layout();
}

void Shader::setCamera(const Camera& camera)
{
    _pipeline_desciptor->camera().assign(camera);
}

const sp<PipelineDescriptor>& Shader::pipelineDesciptor() const
{
    return _pipeline_desciptor;
}

sp<PipelineBindings> Shader::makeBindings(Buffer vertexBuffer, const Enum::DrawMode drawMode, const Enum::DrawProcedure drawProcedure, const Vector<std::pair<uint32_t, sp<Uploader>>>& uploaders) const
{
    return sp<PipelineBindings>::make(drawMode, drawProcedure, std::move(vertexBuffer), _pipeline_desciptor, makeDivivedBuffers(_pipeline_desciptor->layout(), uploaders));
}

Shader::BUILDER_IMPL::BUILDER_IMPL(BeanFactory& factory, const document& manifest, sp<Builder<Camera>> camera, Optional<Vector<StageManifest>> stages, Optional<SnippetManifest> snippets)
    : _factory(factory), _manifest(manifest), _stages(stages ? std::move(stages.value()) : factory.makeBuilderListObject<StageManifest>(manifest, "stage")),
      _snippets(snippets ? std::move(snippets.value()) : factory.makeBuilderList<Snippet>(manifest, "snippet")), _camera(camera ? std::move(camera) : factory.getBuilder<Camera>(manifest, constants::CAMERA)),
      _configuration(factory, manifest)
{
}

sp<Shader> Shader::BUILDER_IMPL::build(const Scope& args)
{
    sp<PipelineBuildingContext> buildingContext = makePipelineBuildingContext(args);
    buildingContext->loadManifest(_manifest, _factory, args);

    sp<Snippet> snippet;
    for(const sp<Builder<Snippet>>& i : _snippets)
        snippet = SnippetComposite::compose(std::move(snippet), i->build(args));

    const sp<Camera> camera = _camera.build(args);
    PipelineDescriptor::Configuration configuration = _configuration.build(args);
    configuration._snippet = std::move(snippet);
    return sp<Shader>::make(sp<PipelineDescriptor>::make(camera ? *camera : Camera::createDefaultCamera(), std::move(buildingContext), std::move(configuration)));
}

sp<PipelineBuildingContext> Shader::BUILDER_IMPL::makePipelineBuildingContext(const Scope& args) const
{
    sp<PipelineBuildingContext> context = sp<PipelineBuildingContext>::make();
    const auto [vertexOpt, vertexIndex] = findStageManifest(Enum::SHADER_STAGE_BIT_VERTEX, _stages);
    const auto [fragmentOpt, fragmentIndex] = findStageManifest(Enum::SHADER_STAGE_BIT_FRAGMENT, _stages);
    const auto [computeOpt, computeIndex] = findStageManifest(Enum::SHADER_STAGE_BIT_COMPUTE, _stages);
    if(vertexOpt || fragmentOpt || !computeOpt)
    {
        const Global<StringTable> globalStringTable;
        document documentNone = Global<Constants>()->DOCUMENT_NONE;
        context->addStage(vertexOpt ? std::move(*vertexOpt->_source->build(args)) : std::move(globalStringTable->getString("shaders", "default.vert", true).value()), documentNone, Enum::SHADER_STAGE_BIT_VERTEX, Enum::SHADER_STAGE_BIT_NONE);
        context->addStage(fragmentOpt ? std::move(*fragmentOpt->_source->build(args)) : std::move(globalStringTable->getString("shaders", "texture.frag", true).value()), std::move(documentNone), Enum::SHADER_STAGE_BIT_FRAGMENT, Enum::SHADER_STAGE_BIT_VERTEX);
    }
    else
        CHECK(computeOpt, "Shader must have at least one stage defined");
    if(computeOpt)
        context->addStage(std::move(*computeOpt->_source->build(args)), computeOpt->_manifest, Enum::SHADER_STAGE_BIT_COMPUTE, fragmentOpt && fragmentIndex < computeIndex ? Enum::SHADER_STAGE_BIT_FRAGMENT : Enum::SHADER_STAGE_BIT_NONE);
    return context;
}

template<> ARK_API Enum::ShaderStageBit StringConvert::eval<Enum::ShaderStageBit>(const String& val)
{
    if(val == "vertex")
        return Enum::SHADER_STAGE_BIT_VERTEX;
    if(val == "fragment")
        return Enum::SHADER_STAGE_BIT_FRAGMENT;
    if(val == "compute")
        return Enum::SHADER_STAGE_BIT_COMPUTE;
    CHECK(val.empty(), "Unknown stage: \"%s\"", val.c_str());
    return Enum::SHADER_STAGE_BIT_NONE;
}

Shader::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _impl(factory, manifest)
{
}

sp<Shader> Shader::BUILDER::build(const Scope& args)
{
    return _impl.build(args);
}

}
