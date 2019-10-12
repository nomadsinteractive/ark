#include "renderer/base/shader.h"

#include "core/base/memory_pool.h"
#include "core/base/string_buffer.h"
#include "core/base/string_table.h"
#include "core/inf/array.h"
#include "core/types/safe_ptr.h"
#include "core/types/global.h"
#include "core/util/documents.h"
#include "core/util/strings.h"

#include "graphics/base/camera.h"
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
    ShaderBuilderImpl(BeanFactory& factory, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext, const String& vertex, const String& fragment, const sp<Camera>& defaultCamera)
        : _factory(factory), _manifest(doc), _render_controller(resourceLoaderContext->renderController()), _vertex(vertex), _fragment(fragment), _default_camera(defaultCamera),
          _camera(factory.getBuilder<Camera>(doc, Constants::Attributes::CAMERA)), _pipeline_bindings_flag(Documents::getAttribute<PipelineBindings::Flag>(_manifest, "flags", PipelineBindings::FLAG_DEFAULT_VALUE)) {
    }

    virtual sp<Shader> build(const Scope& args) override {
        sp<PipelineBuildingContext> buildingContext = sp<PipelineBuildingContext>::make(_render_controller->createPipelineFactory(), _vertex, _fragment, _factory, args, _manifest);
        sp<PipelineLayout> pipelineLayout = sp<PipelineLayout>::make(buildingContext);
        sp<Camera> camera = _camera->build(args);
        return sp<Shader>::make(buildingContext->_pipeline_factory, _render_controller, pipelineLayout, camera ? camera : _default_camera, _pipeline_bindings_flag);
    }

private:
    BeanFactory _factory;
    document _manifest;
    sp<RenderController> _render_controller;

    String _vertex, _fragment;
    sp<Camera> _default_camera;

    SafePtr<Builder<Camera>> _camera;
    PipelineBindings::Flag _pipeline_bindings_flag;

};

}

Shader::Shader(const sp<PipelineFactory> pipelineFactory, const sp<RenderController>& renderController, const sp<PipelineLayout>& pipelineLayout, const sp<Camera>& camera, PipelineBindings::Flag pipelineBindingsFlag)
    : _pipeline_factory(pipelineFactory), _render_controller(renderController), _pipeline_layout(pipelineLayout), _input(_pipeline_layout->input()), _camera(camera ? camera : Camera::getDefaultCamera()), _pipeline_bindings_flag(pipelineBindingsFlag)
{
    _pipeline_layout->initialize(_camera);
}

sp<Builder<Shader>> Shader::fromDocument(BeanFactory& factory, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext, const String& defVertex, const String& defFragment, const sp<Camera>& defaultCamera)
{
    const Global<StringTable> stringTable;
    const sp<Builder<Shader>> shader = factory.getBuilder<Shader>(doc, Constants::Attributes::SHADER);
    return shader ? shader : sp<Builder<Shader>>::adopt(new ShaderBuilderImpl(factory, doc, resourceLoaderContext, stringTable->getString(defVertex, true), stringTable->getString(defFragment, true), defaultCamera));
}

sp<Shader> Shader::fromStringTable(const String& vertex, const String& fragment, const sp<Snippet>& snippet, const sp<ResourceLoaderContext>& resourceLoaderContext)
{
    const Global<StringTable> stringTable;
    const sp<PipelineBuildingContext> buildingContext = sp<PipelineBuildingContext>::make(resourceLoaderContext->renderController()->createPipelineFactory(), stringTable->getString(vertex, true), stringTable->getString(fragment, true));
    if(snippet)
        buildingContext->addSnippet(snippet);

    const sp<PipelineLayout> pipelineLayout = sp<PipelineLayout>::make(buildingContext);
    return sp<Shader>::make(buildingContext->_pipeline_factory, resourceLoaderContext->renderController(), pipelineLayout, nullptr, PipelineBindings::FLAG_DEFAULT_VALUE);
}

std::vector<RenderLayer::UBOSnapshot> Shader::snapshot(Allocator& allocator) const
{
    std::vector<RenderLayer::UBOSnapshot> uboSnapshot;
    for(const sp<PipelineInput::UBO>& i : _input->ubos())
        uboSnapshot.push_back(i->snapshot(allocator));
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

const sp<Camera>& Shader::camera() const
{
    return _camera;
}

const sp<RenderController>& Shader::renderController() const
{
    return _render_controller;
}

const sp<PipelineLayout>& Shader::layout() const
{
    return _pipeline_layout;
}

sp<ShaderBindings> Shader::makeBindings(RenderModel::Mode mode) const
{
    return sp<ShaderBindings>::make(_pipeline_factory, sp<PipelineBindings>::make(mode, _pipeline_layout, _pipeline_bindings_flag), _render_controller);
}

sp<ShaderBindings> Shader::makeBindings(RenderModel::Mode mode, const Buffer& vertex, const Buffer& index) const
{
    return sp<ShaderBindings>::make(_pipeline_factory, sp<PipelineBindings>::make(mode, _pipeline_layout, _pipeline_bindings_flag), _render_controller, vertex, index);
}

Shader::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _factory(factory), _manifest(manifest), _resource_loader_context(resourceLoaderContext), _vertex(factory.getBuilder<String>(manifest, "vertex", "@shaders:default.vert")),
      _fragment(factory.getBuilder<String>(manifest, "fragment", "@shaders:texture.frag")), _snippet(factory.getBuilder<Snippet>(manifest, Constants::Attributes::SNIPPET)),
      _camera(factory.getBuilder<Camera>(manifest, Constants::Attributes::CAMERA)),
      _pipeline_bindings_flag(Documents::getAttribute<PipelineBindings::Flag>(_manifest, "flags", PipelineBindings::FLAG_DEFAULT_VALUE))
{
}

sp<Shader> Shader::BUILDER::build(const Scope& args)
{
    const sp<PipelineBuildingContext> buildingContext = sp<PipelineBuildingContext>::make(_resource_loader_context->renderController()->createPipelineFactory(), _vertex->build(args), _fragment->build(args), _factory, args, _manifest);
    if(_snippet)
        buildingContext->addSnippet(_snippet->build(args));

    const sp<PipelineLayout> pipelineLayout = sp<PipelineLayout>::make(buildingContext);
    return sp<Shader>::make(buildingContext->_pipeline_factory, _resource_loader_context->renderController(), pipelineLayout, _camera->build(args), _pipeline_bindings_flag);
}

}
