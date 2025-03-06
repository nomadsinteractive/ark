#include "renderer/base/shader.h"

#include "core/base/string_table.h"
#include "core/types/global.h"
#include "core/util/string_convert.h"
#include "core/util/documents.h"

#include "graphics/base/camera.h"
#include "renderer/base/varyings.h"

#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/pipeline_configuration.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/inf/renderer_factory.h"
#include "renderer/inf/pipeline_factory.h"

namespace ark {

namespace {

Optional<const Shader::StageManifest&> findStageManifest(const Enum::ShaderStageBit type, const Vector<Shader::StageManifest>& stages)
{
    for(const Shader::StageManifest& i : stages)
        if(type == i._type)
            return {i};
    return {};
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

Shader::Shader(sp<Camera> camera, sp<PipelineFactory> pipelineFactory, sp<RenderController> renderController, sp<PipelineConfiguration> pipelineConfiguration, PipelineDescriptor::Parameters parameters)
    : _camera(camera ? *camera : Camera::createDefaultCamera()), _pipeline_factory(std::move(pipelineFactory)), _render_controller(std::move(renderController)), _pipeline_configuration(std::move(pipelineConfiguration)), _layout(_pipeline_configuration->pipelineLayout()), _descriptor_params(std::move(parameters))
{
    _pipeline_configuration->initialize(*this);
}

sp<Builder<Shader>> Shader::fromDocument(BeanFactory& factory, const document& manifest, const String& defVertex, const String& defFragment, const sp<Camera>& defaultCamera)
{
    if(builder<Shader> shader = factory.getBuilder<Shader>(manifest, constants::SHADER))
        return shader;
    const Global<StringTable> stringTable;
    Vector<StageManifest> stageManifests;
    stageManifests.emplace_back(Enum::SHADER_STAGE_BIT_VERTEX, builder<String>::make<Builder<String>::Prebuilt>(stringTable->getString(defVertex, true)));
    stageManifests.emplace_back(Enum::SHADER_STAGE_BIT_FRAGMENT, builder<String>::make<Builder<String>::Prebuilt>(stringTable->getString(defFragment, true)));
    return builder<Shader>::make<BUILDER_IMPL>(factory, manifest, defaultCamera ? builder<Camera>::make<Builder<Camera>::Prebuilt>(defaultCamera) : nullptr, std::move(stageManifests));
}

sp<RenderLayerSnapshot::BufferObject> Shader::takeBufferSnapshot(const RenderRequest& renderRequest, const bool isComputeStage) const
{
    return _layout->takeBufferSnapshot(renderRequest, isComputeStage);
}

const Camera& Shader::camera() const
{
    return _camera;
}

const sp<PipelineFactory>& Shader::pipelineFactory() const
{
    return _pipeline_factory;
}

const sp<PipelineLayout>& Shader::layout() const
{
    return _layout;
}

const sp<RenderController>& Shader::renderController() const
{
    return _render_controller;
}

void Shader::setCamera(const Camera& camera)
{
    _camera.assign(camera);
}

const sp<PipelineConfiguration>& Shader::pipelineConfiguration() const
{
    return _pipeline_configuration;
}

const PipelineDescriptor::Parameters& Shader::descriptorParams() const
{
    return _descriptor_params;
}

sp<PipelineBindings> Shader::makeBindings(Buffer vertices, Enum::RenderMode mode, Enum::DrawProcedure drawProcedure, const Map<uint32_t, sp<Uploader>>& uploaders) const
{
    return sp<PipelineBindings>::make(std::move(vertices), _pipeline_factory, sp<PipelineDescriptor>::make(mode, drawProcedure, _descriptor_params, _pipeline_configuration), makeDivivedBuffers(uploaders));
}

Map<uint32_t, Buffer> Shader::makeDivivedBuffers(const Map<uint32_t, sp<Uploader>>& uploaders) const
{
    Map<uint32_t, Buffer> dividedBuffers;
    for(const auto& [divisor, _] : _layout->streamLayouts())
        if(divisor != 0)
        {
            CHECK(dividedBuffers.find(divisor) == dividedBuffers.end(), "Duplicated stream divisor: %d", divisor);
            const auto iter = uploaders.find(divisor);
            dividedBuffers.insert(std::make_pair(divisor, _render_controller->makeVertexBuffer(Buffer::USAGE_BIT_DYNAMIC, iter != uploaders.end() ? iter->second : nullptr)));
        }
    return dividedBuffers;
}

Shader::BUILDER_IMPL::BUILDER_IMPL(BeanFactory& factory, const document& manifest, sp<Builder<Camera>> camera, Optional<Vector<StageManifest>> stages, Optional<SnippetManifest> snippets)
    : _factory(factory), _manifest(manifest), _stages(stages ? std::move(stages.value()) : factory.makeBuilderListObject<StageManifest>(manifest, "stage")),
      _snippets(snippets ? std::move(snippets.value()) : factory.makeBuilderList<Snippet>(manifest, "snippet")), _camera(camera ? std::move(camera) : factory.getBuilder<Camera>(manifest, constants::CAMERA)),
      _parameters(factory, manifest)
{
}

sp<Shader> Shader::BUILDER_IMPL::build(const Scope& args)
{
    sp<PipelineBuildingContext> buildingContext = makePipelineBuildingContext(args);
    buildingContext->loadManifest(_manifest, _factory, args);

    sp<PipelineConfiguration> pipelineLayout = sp<PipelineConfiguration>::make(std::move(buildingContext));
    for(const sp<Builder<Snippet>>& i : _snippets)
        pipelineLayout->addSnippet(i->build(args));

    const sp<RenderController>& renderController = Ark::instance().renderController();
    return sp<Shader>::make(_camera.build(args), renderController->createPipelineFactory(), renderController, std::move(pipelineLayout), _parameters.build(args));
}

sp<PipelineBuildingContext> Shader::BUILDER_IMPL::makePipelineBuildingContext(const Scope& args) const
{
    sp<PipelineBuildingContext> context = sp<PipelineBuildingContext>::make();
    const Optional<const StageManifest&> vertexOpt = findStageManifest(Enum::SHADER_STAGE_BIT_VERTEX, _stages);
    const Optional<const StageManifest&> fragmentOpt = findStageManifest(Enum::SHADER_STAGE_BIT_FRAGMENT, _stages);
    const Optional<const StageManifest&> computeOpt = findStageManifest(Enum::SHADER_STAGE_BIT_COMPUTE, _stages);
    if(vertexOpt || fragmentOpt || !computeOpt)
    {
        const Global<StringTable> globalStringTable;
        document documentNone = Global<Constants>()->DOCUMENT_NONE;
        context->addStage(vertexOpt ? vertexOpt->_source->build(args) : globalStringTable->getString("shaders", "default.vert", true), documentNone, Enum::SHADER_STAGE_BIT_VERTEX, Enum::SHADER_STAGE_BIT_NONE);
        context->addStage(fragmentOpt ? fragmentOpt->_source->build(args) : globalStringTable->getString("shaders", "texture.frag", true), std::move(documentNone), Enum::SHADER_STAGE_BIT_FRAGMENT, Enum::SHADER_STAGE_BIT_VERTEX);
    }
    else
        CHECK(computeOpt, "Shader must have at least one stage defined");
    if(computeOpt)
        context->addStage(computeOpt->_source->build(args), computeOpt->_manifest, Enum::SHADER_STAGE_BIT_COMPUTE, fragmentOpt ? Enum::SHADER_STAGE_BIT_FRAGMENT : Enum::SHADER_STAGE_BIT_NONE);
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
