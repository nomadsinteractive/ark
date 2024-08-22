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

class ShaderBuilderImpl final : public Builder<Shader> {
public:
    ShaderBuilderImpl(BeanFactory& factory, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext, sp<String> vertex, sp<String> fragment, sp<Camera> defaultCamera)
        : _factory(factory), _manifest(doc), _render_controller(resourceLoaderContext->renderController()), _vertex(std::move(vertex)), _fragment(std::move(fragment)), _default_camera(defaultCamera ? std::move(defaultCamera) : Camera::getDefaultCamera()),
          _camera(factory.getBuilder<Camera>(doc, constants::CAMERA)), _parameters(factory, doc, resourceLoaderContext) {
    }

    sp<Shader> build(const Scope& args) override {
        const sp<Camera> camera = _camera->build(args);
        sp<PipelineBuildingContext> buildingContext = sp<PipelineBuildingContext>::make(_render_controller, camera ? camera : _default_camera, _vertex, _fragment);
        sp<Snippet> snippet = buildingContext->makePipelineSnippet();
        buildingContext->loadManifest(_manifest, _factory, args);
        return sp<Shader>::make(_render_controller->createPipelineFactory(), _render_controller, sp<PipelineLayout>::make(buildingContext, std::move(snippet)), _parameters.build(args));
    }

private:
    BeanFactory _factory;
    document _manifest;
    sp<RenderController> _render_controller;

    sp<String> _vertex, _fragment;
    sp<Camera> _default_camera;

    SafePtr<Builder<Camera>> _camera;
    PipelineDescriptor::Parameters::BUILDER _parameters;
};

Optional<const Shader::StageManifest&> findStageManifest(Enum::ShaderStageBit type, const std::vector<Shader::StageManifest>& stages)
{
    for(const Shader::StageManifest& i : stages)
        if(type == i._type)
            return {i};
    return {};
}

}

Shader::StageManifest::StageManifest(BeanFactory& factory, const document& manifest)
    : _type(Documents::ensureAttribute<Enum::ShaderStageBit>(manifest, constants::TYPE)), _source(factory.ensureBuilder<String>(manifest, constants::SRC))
{
}

Shader::Shader(sp<PipelineFactory> pipelineFactory, sp<RenderController> renderController, sp<PipelineLayout> pipelineLayout, PipelineDescriptor::Parameters bindingParams)
    : _pipeline_factory(std::move(pipelineFactory)), _render_controller(std::move(renderController)), _pipeline_layout(std::move(pipelineLayout)), _pipeline_input(_pipeline_layout->input()), _binding_params(std::move(bindingParams))
{
}

sp<Builder<Shader>> Shader::fromDocument(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext, const String& defVertex, const String& defFragment, const sp<Camera>& defaultCamera)
{
    const Global<StringTable> stringTable;
    builder<Shader> shader = factory.getBuilder<Shader>(manifest, constants::SHADER);
    return shader ? shader : builder<Shader>::make<ShaderBuilderImpl>(factory, manifest, resourceLoaderContext, stringTable->getString(defVertex, true), stringTable->getString(defFragment, true), defaultCamera);
}

sp<RenderLayerSnapshot::BufferObject> Shader::takeBufferSnapshot(const RenderRequest& renderRequest, bool isComputeStage) const
{
    std::vector<RenderLayerSnapshot::UBOSnapshot> uboSnapshot;
    for(const sp<PipelineInput::UBO>& i : _pipeline_input->ubos())
        if(isComputeStage ? i->stages().has(Enum::SHADER_STAGE_BIT_COMPUTE) : i->stages() != Enum::SHADER_STAGE_BIT_COMPUTE)
            uboSnapshot.push_back(i->snapshot(renderRequest));

    std::vector<std::pair<uint32_t, Buffer::Snapshot>> ssboSnapshot;
    for(const PipelineInput::SSBO& i : _pipeline_input->ssbos())
        if(isComputeStage ? i._stages.has(Enum::SHADER_STAGE_BIT_COMPUTE) : i._stages != Enum::SHADER_STAGE_BIT_COMPUTE)
            ssboSnapshot.emplace_back(i._binding, i._buffer.snapshot());

    return sp<RenderLayerSnapshot::BufferObject>::make(RenderLayerSnapshot::BufferObject{std::move(uboSnapshot), std::move(ssboSnapshot)});
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

sp<PipelineBindings> Shader::makeBindings(Buffer vertices, Enum::RenderMode mode, Enum::DrawProcedure drawProcedure, const std::map<uint32_t, sp<Uploader>>& uploaders) const
{
    return sp<PipelineBindings>::make(std::move(vertices), _pipeline_factory, sp<PipelineDescriptor>::make(mode, drawProcedure, _binding_params, _pipeline_layout), makeDivivedBuffers(uploaders));
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

    for(const sp<Builder<Snippet>>& i : _snippets)
        buildingContext->addSnippet(i->build(args));
    sp<Snippet> snippet = buildingContext->makePipelineSnippet();

    buildingContext->loadManifest(_manifest, _factory, args);
    return sp<Shader>::make(_render_controller->createPipelineFactory(), _render_controller, sp<PipelineLayout>::make(buildingContext, std::move(snippet)), _parameters.build(args));
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
        context->addStage(vertexOpt ? vertexOpt->_source->build(args) : globalStringTable->getString("shaders", "default.vert", true), Enum::SHADER_STAGE_BIT_VERTEX, Enum::SHADER_STAGE_BIT_NONE);
        context->addStage(fragmentOpt ? fragmentOpt->_source->build(args) : globalStringTable->getString("shaders", "texture.frag", true), Enum::SHADER_STAGE_BIT_FRAGMENT, Enum::SHADER_STAGE_BIT_VERTEX);
    }
    else
        CHECK(computeOpt, "Shader must have at least one stage defined");
    if(computeOpt)
        context->addStage(computeOpt->_source->build(args), Enum::SHADER_STAGE_BIT_COMPUTE, Enum::SHADER_STAGE_BIT_NONE);
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
