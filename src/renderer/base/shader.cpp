#include "renderer/base/shader.h"

#include "core/base/string_table.h"
#include "core/types/safe_ptr.h"
#include "core/types/global.h"
#include "core/util/string_convert.h"
#include "core/util/documents.h"

#include "graphics/base/camera.h"
#include "renderer/base/varyings.h"

#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/inf/renderer_factory.h"
#include "renderer/inf/pipeline_factory.h"

namespace ark {

namespace {

Optional<const Shader::StageManifest&> findStageManifest(Enum::ShaderStageBit type, const std::vector<Shader::StageManifest>& stages)
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

Shader::Shader(sp<PipelineFactory> pipelineFactory, sp<RenderController> renderController, sp<PipelineLayout> pipelineLayout, PipelineDescriptor::Parameters bindingParams)
    : _pipeline_factory(std::move(pipelineFactory)), _render_controller(std::move(renderController)), _pipeline_layout(std::move(pipelineLayout)), _pipeline_input(_pipeline_layout->input()), _descriptor_params(std::move(bindingParams))
{
    _pipeline_layout->initialize(*this);
}

sp<Builder<Shader>> Shader::fromDocument(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext, const String& defVertex, const String& defFragment, const sp<Camera>& defaultCamera)
{
    if(builder<Shader> shader = factory.getBuilder<Shader>(manifest, constants::SHADER))
        return shader;
    const Global<StringTable> stringTable;
    std::vector<StageManifest> stageManifests;
    stageManifests.emplace_back(Enum::SHADER_STAGE_BIT_VERTEX, builder<String>::make<Builder<String>::Prebuilt>(stringTable->getString(defVertex, true)));
    stageManifests.emplace_back(Enum::SHADER_STAGE_BIT_FRAGMENT, builder<String>::make<Builder<String>::Prebuilt>(stringTable->getString(defFragment, true)));
    return builder<Shader>::make<BUILDER_IMPL>(factory, manifest, resourceLoaderContext, defaultCamera ? builder<Camera>::make<Builder<Camera>::Prebuilt>(defaultCamera) : nullptr, std::move(stageManifests));
}

sp<RenderLayerSnapshot::BufferObject> Shader::takeBufferSnapshot(const RenderRequest& renderRequest, bool isComputeStage) const
{
    return _pipeline_input->takeBufferSnapshot(renderRequest, isComputeStage);
}

const sp<PipelineFactory>& Shader::pipelineFactory() const
{
    return _pipeline_factory;
}

const sp<PipelineInput>& Shader::input() const
{
    return _pipeline_input;
}

const sp<RenderController>& Shader::renderController() const
{
    return _render_controller;
}

void Shader::setCamera(const Camera& camera)
{
    _pipeline_input->_camera.assign(camera);
}

const sp<PipelineLayout>& Shader::layout() const
{
    return _pipeline_layout;
}

const PipelineDescriptor::Parameters& Shader::descriptorParams() const
{
    return _descriptor_params;
}

sp<PipelineBindings> Shader::makeBindings(Buffer vertices, Enum::RenderMode mode, Enum::DrawProcedure drawProcedure, const std::map<uint32_t, sp<Uploader>>& uploaders) const
{
    return sp<PipelineBindings>::make(std::move(vertices), _pipeline_factory, sp<PipelineDescriptor>::make(mode, drawProcedure, _descriptor_params, _pipeline_layout), makeDivivedBuffers(uploaders));
}

std::map<uint32_t, Buffer> Shader::makeDivivedBuffers(const std::map<uint32_t, sp<Uploader>>& uploaders) const
{
    std::map<uint32_t, Buffer> dividedBuffers;
    for(const auto& [divisor, _] : _pipeline_input->streamLayouts())
        if(divisor != 0)
        {
            CHECK(dividedBuffers.find(divisor) == dividedBuffers.end(), "Duplicated stream divisor: %d", divisor);
            const auto iter = uploaders.find(divisor);
            dividedBuffers.insert(std::make_pair(divisor, _render_controller->makeVertexBuffer(Buffer::USAGE_BIT_DYNAMIC, iter != uploaders.end() ? iter->second : nullptr)));
        }
    return dividedBuffers;
}

Shader::BUILDER_IMPL::BUILDER_IMPL(BeanFactory& factory, const document& manifest, const ResourceLoaderContext& resourceLoaderContext, sp<Builder<Camera>> camera, Optional<std::vector<StageManifest>> stages, Optional<SnippetManifest> snippets)
    : _factory(factory), _manifest(manifest), _render_controller(resourceLoaderContext.renderController()), _stages(stages ? std::move(stages.value()) : factory.makeBuilderListObject<StageManifest>(manifest, "stage")),
      _snippets(snippets ? std::move(snippets.value()) : factory.makeBuilderList<Snippet>(manifest, "snippet")), _camera(camera ? std::move(camera) : factory.getBuilder<Camera>(manifest, constants::CAMERA)),
      _parameters(factory, manifest, resourceLoaderContext)
{
}

sp<Shader> Shader::BUILDER_IMPL::build(const Scope& args)
{
    sp<PipelineBuildingContext> buildingContext = makePipelineBuildingContext(_camera->build(args), args);
    buildingContext->loadManifest(_manifest, _factory, args);

    sp<PipelineLayout> pipelineLayout = sp<PipelineLayout>::make(std::move(buildingContext));
    for(const sp<Builder<Snippet>>& i : _snippets)
        pipelineLayout->addSnippet(i->build(args));

    return sp<Shader>::make(_render_controller->createPipelineFactory(), _render_controller, std::move(pipelineLayout), _parameters.build(args));
}

sp<PipelineBuildingContext> Shader::BUILDER_IMPL::makePipelineBuildingContext(const sp<Camera>& camera, const Scope& args) const
{
    sp<PipelineBuildingContext> context = sp<PipelineBuildingContext>::make(_render_controller, camera);
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
        context->addStage(computeOpt->_source->build(args), computeOpt->_manifest, Enum::SHADER_STAGE_BIT_COMPUTE, Enum::SHADER_STAGE_BIT_NONE);
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

Shader::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _impl(factory, manifest, resourceLoaderContext)
{
}

sp<Shader> Shader::BUILDER::build(const Scope& args)
{
    return _impl.build(args);
}

}
