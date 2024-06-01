#include "renderer/base/shader.h"

#include "core/base/string_buffer.h"
#include "core/base/string_table.h"
#include "core/inf/array.h"
#include "core/types/safe_ptr.h"
#include "core/types/global.h"
#include "core/util/string_convert.h"
#include "core/util/documents.h"
#include "core/util/strings.h"

#include "graphics/base/camera.h"
#include "graphics/base/v4.h"
#include "renderer/base/varyings.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/inf/renderer_factory.h"
#include "renderer/inf/pipeline_factory.h"
#include "renderer/inf/pipeline.h"

#include "platform/platform.h"

namespace ark {

namespace {

class ShaderBuilderImpl : public Builder<Shader> {
public:
    ShaderBuilderImpl(BeanFactory& factory, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext, sp<String> vertex, sp<String> fragment, sp<Camera> defaultCamera)
        : _factory(factory), _manifest(doc), _render_controller(resourceLoaderContext->renderController()), _vertex(std::move(vertex)), _fragment(std::move(fragment)), _default_camera(defaultCamera ? std::move(defaultCamera) : Camera::getDefaultCamera()),
          _camera(factory.getBuilder<Camera>(doc, constants::CAMERA)), _parameters(factory, doc, resourceLoaderContext) {
    }

    virtual sp<Shader> build(const Scope& args) override {
        sp<PipelineBuildingContext> buildingContext = sp<PipelineBuildingContext>::make(_render_controller, _vertex, _fragment);
        buildingContext->loadManifest(_manifest, _factory, args);
        sp<Camera> camera = _camera->build(args);
        buildingContext->_input->camera().assign(camera ? camera : _default_camera);
        return sp<Shader>::make(_render_controller->createPipelineFactory(), _render_controller, sp<PipelineLayout>::make(buildingContext), _parameters.build(args));
    }

private:
    BeanFactory _factory;
    document _manifest;
    sp<RenderController> _render_controller;

    sp<String> _vertex, _fragment;
    sp<Camera> _default_camera;

    SafePtr<Builder<Camera>> _camera;
    PipelineBindings::Parameters::BUILDER _parameters;

};

}

Shader::Shader(sp<PipelineFactory> pipelineFactory, sp<RenderController> renderController, sp<PipelineLayout> pipelineLayout, PipelineBindings::Parameters bindingParams)
    : _pipeline_factory(std::move(pipelineFactory)), _render_controller(std::move(renderController)), _pipeline_layout(std::move(pipelineLayout)), _pipeline_input(_pipeline_layout->input()), _binding_params(std::move(bindingParams))
{
}

sp<Builder<Shader>> Shader::fromDocument(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext, const String& defVertex, const String& defFragment, const sp<Camera>& defaultCamera)
{
    const Global<StringTable> stringTable;
    sp<Builder<Shader>> shader = factory.getBuilder<Shader>(manifest, constants::SHADER);
    return shader ? shader : sp<Builder<Shader>>::make<ShaderBuilderImpl>(factory, manifest, resourceLoaderContext, stringTable->getString(defVertex, true), stringTable->getString(defFragment, true), defaultCamera);
}

std::vector<RenderLayerSnapshot::UBOSnapshot> Shader::takeUBOSnapshot(const RenderRequest& renderRequest) const
{
    std::vector<RenderLayerSnapshot::UBOSnapshot> uboSnapshot;
    for(const sp<PipelineInput::UBO>& i : _pipeline_input->ubos())
        uboSnapshot.push_back(i->snapshot(renderRequest));
    return uboSnapshot;
}

std::vector<std::pair<uint32_t, Buffer::Snapshot>> Shader::takeSSBOSnapshot(const RenderRequest& /*renderRequest*/) const
{
    std::vector<std::pair<uint32_t, Buffer::Snapshot>> ssboSnapshot;
    for(const PipelineInput::SSBO& i : _pipeline_input->ssbos())
        ssboSnapshot.push_back(std::make_pair(i._binding, i._buffer.snapshot()));
    return ssboSnapshot;
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

sp<ShaderBindings> Shader::makeBindings(Buffer vertices, Enum::RenderMode mode, Enum::DrawProcedure drawProcedure, const std::map<uint32_t, sp<Uploader>>& uploaders) const
{
    return sp<ShaderBindings>::make(std::move(vertices), _pipeline_factory, sp<PipelineBindings>::make(mode, drawProcedure, _binding_params, _pipeline_layout), makeDivivedBuffers(uploaders));
}

std::map<uint32_t, Buffer> Shader::makeDivivedBuffers(const std::map<uint32_t, sp<Uploader>>& uploaders) const
{
    std::map<uint32_t, Buffer> dividedBuffers;
    for(const auto& i : _pipeline_input->streams())
    {
        uint32_t divisor = i.first;
        if(divisor != 0)
        {
            CHECK(dividedBuffers.find(divisor) == dividedBuffers.end(), "Duplicated stream divisor: %d", divisor);
            const auto iter = uploaders.find(divisor);
            dividedBuffers.insert(std::make_pair(divisor, _render_controller->makeVertexBuffer(Buffer::USAGE_DYNAMIC, iter != uploaders.end() ? iter->second : nullptr)));
        }
    }
    return dividedBuffers;
}

Shader::BUILDER_IMPL::BUILDER_IMPL(BeanFactory& factory, const document& manifest, const ResourceLoaderContext& resourceLoaderContext, sp<Builder<Camera>> camera, Optional<StageManifest> stages, Optional<SnippetManifest> snippets)
    : _factory(factory), _manifest(manifest), _render_controller(resourceLoaderContext.renderController()), _stages(stages ? std::move(stages.value()) : loadStages(factory, manifest)),
      _snippets(snippets ? std::move(snippets.value()) : factory.makeBuilderList<Snippet>(manifest, "snippet")), _camera(camera ? std::move(camera) : factory.getBuilder<Camera>(manifest, constants::CAMERA)),
      _parameters(factory, manifest, resourceLoaderContext)
{
}

sp<Shader> Shader::BUILDER_IMPL::build(const Scope& args)
{
    sp<PipelineBuildingContext> buildingContext = makePipelineBuildingContext(args);
    buildingContext->loadManifest(_manifest, _factory, args);

    for(const sp<Builder<Snippet>>& i : _snippets)
        buildingContext->addSnippet(i->build(args));

    const sp<Camera> camera = _camera->build(args);
    buildingContext->_input->camera().assign(camera ? camera : Camera::getDefaultCamera());
    return sp<Shader>::make(_render_controller->createPipelineFactory(), _render_controller, sp<PipelineLayout>::make(buildingContext), _parameters.build(args));
}

Shader::StageManifest Shader::BUILDER_IMPL::loadStages(BeanFactory& factory, const document& manifest) const
{
    Shader::StageManifest stages;

    for(const document& i : manifest->children("stage"))
    {
        PipelineInput::ShaderStage type = Documents::ensureAttribute<PipelineInput::ShaderStage>(i, constants::TYPE);
        CHECK(stages.find(type) == stages.end(), "Stage duplicated: %s", Documents::getAttribute(i, constants::TYPE).c_str());
        stages[type] = factory.ensureBuilder<String>(i, constants::SRC);
    }

    if(stages.empty())
    {
        stages[PipelineInput::SHADER_STAGE_VERTEX] = factory.getBuilder<String>(manifest, "vertex", "@shaders:default.vert");
        stages[PipelineInput::SHADER_STAGE_FRAGMENT] = factory.getBuilder<String>(manifest, "fragment", "@shaders:texture.frag");
    }

    return stages;
}

sp<PipelineBuildingContext> Shader::BUILDER_IMPL::makePipelineBuildingContext(const Scope& args) const
{
    sp<PipelineBuildingContext> context = sp<PipelineBuildingContext>::make(_render_controller);
    PipelineInput::ShaderStage prestage = PipelineInput::SHADER_STAGE_NONE;
    for(const auto& i : _stages)
    {
        context->addStage(i.second->build(args), i.first, prestage);
        prestage = i.first;
    }
    return context;
}

template<> ARK_API PipelineInput::ShaderStage StringConvert::eval<PipelineInput::ShaderStage>(const String& val)
{
    if(val == "vertex")
        return PipelineInput::SHADER_STAGE_VERTEX;
    if(val == "fragment")
        return PipelineInput::SHADER_STAGE_FRAGMENT;
    if(val == "compute")
        return PipelineInput::SHADER_STAGE_COMPUTE;
    CHECK(val.empty(), "Unknown stage: \"%s\"", val.c_str());
    return PipelineInput::SHADER_STAGE_NONE;
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
