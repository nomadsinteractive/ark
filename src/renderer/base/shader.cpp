#include "renderer/base/shader.h"

#include "core/base/memory_pool.h"
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
          _camera(factory.getBuilder<Camera>(doc, Constants::Attributes::CAMERA)), _parameters(factory, doc, resourceLoaderContext) {
    }

    virtual sp<Shader> build(const Scope& args) override {
        sp<PipelineBuildingContext> buildingContext = sp<PipelineBuildingContext>::make(_render_controller, _vertex, _fragment);
        buildingContext->loadManifest(_manifest, _factory, args);
        sp<Camera> camera = _camera->build(args);
        return sp<Shader>::make(_render_controller->createPipelineFactory(), _render_controller, sp<PipelineLayout>::make(buildingContext, camera ? camera : _default_camera), _parameters.build(args));
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
    : _pipeline_factory(std::move(pipelineFactory)), _render_controller(std::move(renderController)), _pipeline_layout(std::move(pipelineLayout)), _input(_pipeline_layout->input()), _binding_params(std::move(bindingParams))
{
}

sp<Builder<Shader>> Shader::fromDocument(BeanFactory& factory, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext, const String& defVertex, const String& defFragment, const sp<Camera>& defaultCamera)
{
    const Global<StringTable> stringTable;
    const sp<Builder<Shader>> shader = factory.getBuilder<Shader>(doc, Constants::Attributes::SHADER);
    return shader ? shader : sp<Builder<Shader>>::make<ShaderBuilderImpl>(factory, doc, resourceLoaderContext, stringTable->getString(defVertex, true), stringTable->getString(defFragment, true), defaultCamera);
}

sp<Shader> Shader::fromStringTable(const String& vertex, const String& fragment, const sp<Snippet>& snippet, const sp<ResourceLoaderContext>& resourceLoaderContext)
{
    const Global<StringTable> stringTable;
    const sp<PipelineBuildingContext> buildingContext = sp<PipelineBuildingContext>::make(resourceLoaderContext->renderController(), stringTable->getString(vertex, true), stringTable->getString(fragment, true));
    if(snippet)
        buildingContext->addSnippet(snippet);

    const sp<RenderController>& renderController = resourceLoaderContext->renderController();
    return sp<Shader>::make(renderController->createPipelineFactory(), renderController, sp<PipelineLayout>::make(buildingContext, Camera::getDefaultCamera()), PipelineBindings::Parameters(Rect(), PipelineBindings::FragmentTestTable(), PipelineBindings::FLAG_DEFAULT_VALUE));
}

std::vector<RenderLayerSnapshot::UBOSnapshot> Shader::takeUBOSnapshot(const RenderRequest& renderRequest) const
{
    std::vector<RenderLayerSnapshot::UBOSnapshot> uboSnapshot;
    for(const sp<PipelineInput::UBO>& i : _input->ubos())
        uboSnapshot.push_back(i->snapshot(renderRequest));
    return uboSnapshot;
}

std::vector<std::pair<uint32_t, Buffer::Snapshot>> Shader::takeSSBOSnapshot(const RenderRequest& /*renderRequest*/) const
{
    std::vector<std::pair<uint32_t, Buffer::Snapshot>> ssboSnapshot;
    for(const PipelineInput::SSBO& i : _input->ssbos())
        ssboSnapshot.push_back(std::make_pair(i._binding, i._buffer.snapshot()));
    return ssboSnapshot;
}

const sp<PipelineFactory>& Shader::pipelineFactory() const
{
    return _pipeline_factory;
}

const sp<PipelineInput>& Shader::input() const
{
    return _input;
}

const sp<RenderController>& Shader::renderController() const
{
    return _render_controller;
}

const sp<PipelineLayout>& Shader::layout() const
{
    return _pipeline_layout;
}

sp<ShaderBindings> Shader::makeBindings(Buffer vertices, ModelLoader::RenderMode mode, PipelineBindings::RenderProcedure renderProcedure) const
{
    return sp<ShaderBindings>::make(std::move(vertices), _pipeline_factory, sp<PipelineBindings>::make(mode, renderProcedure, _binding_params, _pipeline_layout), _render_controller);
}

Shader::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _factory(factory), _manifest(manifest), _resource_loader_context(resourceLoaderContext), _stages(loadStages(factory, manifest)),
      _snippets(factory.getBuilderList<Snippet>(manifest, "snippet")), _camera(factory.getBuilder<Camera>(manifest, Constants::Attributes::CAMERA)),
      _parameters(factory, manifest, resourceLoaderContext)
{
}

sp<Shader> Shader::BUILDER::build(const Scope& args)
{
    const sp<PipelineBuildingContext> buildingContext = makePipelineBuildingContext(args);
    buildingContext->loadManifest(_manifest, _factory, args);

    for(const sp<Builder<Snippet>>& i : _snippets)
        buildingContext->addSnippet(i->build(args));

    const sp<RenderController>& renderController = _resource_loader_context->renderController();
    const sp<Camera> camera = _camera->build(args);
    return sp<Shader>::make(renderController->createPipelineFactory(), renderController, sp<PipelineLayout>::make(buildingContext, camera ? camera : Camera::getDefaultCamera()), _parameters.build(args));
}

std::map<PipelineInput::ShaderStage, sp<Builder<String>>> Shader::BUILDER::loadStages(BeanFactory& factory, const document& manifest) const
{
    std::map<PipelineInput::ShaderStage, sp<Builder<String>>> stages;

    for(const document& i : manifest->children("stage"))
    {
        PipelineInput::ShaderStage type = Documents::ensureAttribute<PipelineInput::ShaderStage>(i, Constants::Attributes::TYPE);
        DCHECK(stages.find(type) == stages.end(), "Stage duplicated: %s", Documents::getAttribute(i, Constants::Attributes::TYPE).c_str());
        stages[type] = factory.ensureBuilder<String>(i, Constants::Attributes::SRC);
    }

    if(stages.empty())
    {
        stages[PipelineInput::SHADER_STAGE_VERTEX] = factory.getBuilder<String>(manifest, "vertex", "@shaders:default.vert");
        stages[PipelineInput::SHADER_STAGE_FRAGMENT] = factory.getBuilder<String>(manifest, "fragment", "@shaders:texture.frag");
    }

    return stages;
}

sp<PipelineBuildingContext> Shader::BUILDER::makePipelineBuildingContext(const Scope& args) const
{
    sp<PipelineBuildingContext> context = sp<PipelineBuildingContext>::make(_resource_loader_context->renderController());
    PipelineInput::ShaderStage prestage = PipelineInput::SHADER_STAGE_NONE;
    for(const auto& i : _stages)
    {
        context->addStage(i.second->build(args), i.first, prestage);
        prestage = i.first;
    }
    return context;
}

template<> ARK_API PipelineInput::ShaderStage StringConvert::to<String, PipelineInput::ShaderStage>(const String& val)
{
    if(val == "vertex")
        return PipelineInput::SHADER_STAGE_VERTEX;
    if(val == "fragment")
        return PipelineInput::SHADER_STAGE_FRAGMENT;
    if(val == "compute")
        return PipelineInput::SHADER_STAGE_COMPUTE;
    DCHECK(val.empty(), "Unknown stage: '%s'", val.c_str());
    return PipelineInput::SHADER_STAGE_NONE;
}


}
