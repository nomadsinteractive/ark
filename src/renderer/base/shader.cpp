#include "renderer/base/shader.h"

#include "core/base/memory_pool.h"
#include "core/base/string_buffer.h"
#include "core/base/string_table.h"
#include "core/inf/array.h"
#include "core/types/safe_ptr.h"
#include "core/types/global.h"
#include "core/util/conversions.h"
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
    ShaderBuilderImpl(BeanFactory& factory, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext, sp<String> vertex, sp<String> fragment, const sp<Camera>& defaultCamera)
        : _factory(factory), _manifest(doc), _render_controller(resourceLoaderContext->renderController()), _vertex(std::move(vertex)), _fragment(std::move(fragment)), _default_camera(defaultCamera),
          _camera(factory.getBuilder<Camera>(doc, Constants::Attributes::CAMERA)), _pipeline_bindings_scissor(factory.getBuilder<Vec4>(_manifest, "scissor")),
          _pipeline_bindings_flags(Documents::getAttribute<PipelineBindings::Flag>(_manifest, "flags", PipelineBindings::FLAG_DEFAULT_VALUE)) {
    }

    virtual sp<Shader> build(const Scope& args) override {
        sp<PipelineBuildingContext> buildingContext = sp<PipelineBuildingContext>::make(_vertex, _fragment);
        buildingContext->loadManifest(_manifest, _factory, args);
        sp<Camera> camera = _camera->build(args);
        const sp<Vec4> scissor = _pipeline_bindings_scissor->build(args);
        return sp<Shader>::make(_render_controller->createPipelineFactory(), _render_controller, sp<PipelineLayout>::make(buildingContext), std::vector<sp<SnippetDraw>>(), camera ? camera : _default_camera, scissor ? Rect(scissor->val()) : Rect(), _pipeline_bindings_flags);
    }

private:
    BeanFactory _factory;
    document _manifest;
    sp<RenderController> _render_controller;

    sp<String> _vertex, _fragment;
    sp<Camera> _default_camera;

    SafePtr<Builder<Camera>> _camera;
    SafePtr<Builder<Vec4>> _pipeline_bindings_scissor;
    uint32_t _pipeline_bindings_flags;

};

}

Shader::Shader(sp<PipelineFactory> pipelineFactory, sp<RenderController> renderController, sp<PipelineLayout> pipelineLayout, std::vector<sp<SnippetDraw>> drawSnippets,
               const sp<Camera>& camera, const Rect& pipelineBindingsScissor, uint32_t pipelineBindingsFlag)
    : _pipeline_factory(std::move(pipelineFactory)), _render_controller(std::move(renderController)), _pipeline_layout(std::move(pipelineLayout)), _input(_pipeline_layout->input()), _camera(camera ? camera : Camera::getDefaultCamera()),
      _draw_snippets(std::move(drawSnippets)), _pipeline_bindings_scissor(_render_controller->renderEngine()->toRendererScissor(pipelineBindingsScissor)), _pipeline_bindings_flag(pipelineBindingsFlag)
{
    _pipeline_layout->initialize(_camera);
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
    const sp<PipelineBuildingContext> buildingContext = sp<PipelineBuildingContext>::make(stringTable->getString(vertex, true), stringTable->getString(fragment, true));
    if(snippet)
        buildingContext->addSnippet(snippet);

    const sp<RenderController>& renderController = resourceLoaderContext->renderController();
    return sp<Shader>::make(renderController->createPipelineFactory(), renderController, sp<PipelineLayout>::make(buildingContext), std::vector<sp<SnippetDraw>>(), nullptr, Rect(), PipelineBindings::FLAG_DEFAULT_VALUE);
}

std::vector<RenderLayer::UBOSnapshot> Shader::snapshot(const RenderRequest& renderRequest) const
{
    std::vector<RenderLayer::UBOSnapshot> uboSnapshot;
    for(const sp<PipelineInput::UBO>& i : _input->ubos())
        uboSnapshot.push_back(i->snapshot(renderRequest));
    return uboSnapshot;
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

sp<ShaderBindings> Shader::makeBindings(ModelLoader::RenderMode mode, PipelineBindings::RenderProcedure renderProcedure) const
{
    const sp<ShaderBindings> shaderBindings = sp<ShaderBindings>::make(_pipeline_factory, sp<PipelineBindings>::make(PipelineBindings::Parameters(mode, renderProcedure, _pipeline_bindings_scissor, _pipeline_bindings_flag), _pipeline_layout), _render_controller);
    for(const sp<SnippetDraw>& i : _draw_snippets)
        shaderBindings->addSnippetDraw(i);
    return shaderBindings;
}

Shader::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _factory(factory), _manifest(manifest), _resource_loader_context(resourceLoaderContext), _stages(loadStages(factory, manifest)),
      _draw_snippets(factory.getBuilderList<SnippetDraw>(manifest, "snippet-draw")), _snippet(factory.getBuilder<Snippet>(manifest, Constants::Attributes::SNIPPET)),
      _camera(factory.getBuilder<Camera>(manifest, Constants::Attributes::CAMERA)), _pipeline_bindings_scissor(factory.getBuilder<Vec4>(_manifest, "scissor")),
      _pipeline_bindings_flags(Documents::getAttribute<PipelineBindings::Flag>(_manifest, "flags", PipelineBindings::FLAG_DEFAULT_VALUE))
{
}

sp<Shader> Shader::BUILDER::build(const Scope& args)
{
    const sp<PipelineBuildingContext> buildingContext = makePipelineBuildingContext(args);
    buildingContext->loadManifest(_manifest, _factory, args);
    if(_snippet)
        buildingContext->addSnippet(_snippet->build(args));

    std::vector<sp<SnippetDraw>> drawSnippets;
    for(const sp<Builder<SnippetDraw>>& i : _draw_snippets)
        drawSnippets.push_back(i->build(args));

    const sp<Vec4> scissor = _pipeline_bindings_scissor->build(args);
    const sp<RenderController>& renderController = _resource_loader_context->renderController();
    return sp<Shader>::make(renderController->createPipelineFactory(), renderController, sp<PipelineLayout>::make(buildingContext), std::move(drawSnippets), _camera->build(args), scissor ? Rect(scissor->val()) : Rect(), _pipeline_bindings_flags);
}

std::map<Shader::Stage, sp<Builder<String>>> Shader::BUILDER::loadStages(BeanFactory& factory, const document& manifest) const
{
    std::map<Shader::Stage, sp<Builder<String>>> stages;

    for(const document& i : manifest->children("stage"))
    {
        Shader::Stage type = Documents::ensureAttribute<Shader::Stage>(i, Constants::Attributes::TYPE);
        DCHECK(stages.find(type) == stages.end(), "Stage duplicated: %s", Documents::getAttribute(i, Constants::Attributes::TYPE).c_str());
        stages[type] = factory.ensureBuilder<String>(i, Constants::Attributes::SRC);
    }

    if(stages.empty())
    {
        stages[Shader::SHADER_STAGE_VERTEX] = factory.getBuilder<String>(manifest, "vertex", "@shaders:default.vert");
        stages[Shader::SHADER_STAGE_FRAGMENT] = factory.getBuilder<String>(manifest, "fragment", "@shaders:texture.frag");
    }

    return stages;
}

sp<PipelineBuildingContext> Shader::BUILDER::makePipelineBuildingContext(const Scope& args) const
{
    sp<PipelineBuildingContext> context = sp<PipelineBuildingContext>::make();
    Shader::Stage prestage = Shader::SHADER_STAGE_NONE;
    for(const auto& i : _stages)
    {
        context->addStage(i.second->build(args), i.first, prestage);
        prestage = i.first;
    }
    return context;
}

template<> ARK_API Shader::Stage Conversions::to<String, Shader::Stage>(const String& val)
{
    if(val == "vertex")
        return Shader::SHADER_STAGE_VERTEX;
    if(val == "fragment")
        return Shader::SHADER_STAGE_FRAGMENT;
    if(val == "compute")
        return Shader::SHADER_STAGE_COMPUTE;
    DCHECK(val.empty(), "Unknown stage: '%s'", val.c_str());
    return Shader::SHADER_STAGE_NONE;
}


}
