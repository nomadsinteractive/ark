#include "renderer/base/shader.h"

#include "core/base/memory_pool.h"
#include "core/base/string_buffer.h"
#include "core/base/string_table.h"
#include "core/inf/array.h"
#include "core/impl/flatable/flatable_numeric.h"
#include "core/types/safe_ptr.h"
#include "core/types/global.h"
#include "core/util/documents.h"
#include "core/util/strings.h"

#include "graphics/base/camera.h"
#include "renderer/base/varyings.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/resource_manager.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/inf/renderer_factory.h"
#include "renderer/inf/pipeline_factory.h"

#include "platform/platform.h"

namespace ark {

namespace {

class GLShaderBuilderImpl : public Builder<Shader> {
public:
    GLShaderBuilderImpl(BeanFactory& factory, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext, const String& vertexSrc, const String& fragmentSrc)
        : _factory(factory), _manifest(doc), _render_controller(resourceLoaderContext->renderController()), _vertex_src(vertexSrc), _fragment_src(fragmentSrc),
          _camera(factory.getBuilder<Camera>(doc, Constants::Attributes::CAMERA)) {
    }

    virtual sp<Shader> build(const sp<Scope>& args) override {
        const sp<PipelineBuildingContext> buildingContext = sp<PipelineBuildingContext>::make(_render_controller->createPipelineFactory(), _vertex_src, _fragment_src);
        buildingContext->loadPredefinedParam(_factory, args, _manifest);

        const sp<PipelineLayout> pipelineLayout = sp<PipelineLayout>::make(_render_controller, buildingContext);
        return sp<Shader>::make(buildingContext->_shader, pipelineLayout, _camera->build(args));
    }

private:
    BeanFactory _factory;
    document _manifest;
    sp<RenderController> _render_controller;

    String _vertex_src, _fragment_src;

    SafePtr<Builder<Camera>> _camera;
};

}

Shader::Shader(const sp<Stub>& stub, const sp<PipelineLayout>& pipelineLayout, const sp<Camera>& camera)
    : _stub(stub), _pipeline_layout(pipelineLayout), _input(_pipeline_layout->input()), _camera(camera ? camera : Camera::getMainCamera())
{
    _pipeline_layout->initialize(_camera);
}

sp<Builder<Shader>> Shader::fromDocument(BeanFactory& factory, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext, const String& defVertex, const String& defFragment)
{
    const Global<StringTable> stringTable;
    const sp<Builder<Shader>> shader = factory.getBuilder<Shader>(doc, Constants::Attributes::SHADER);
    return shader ? shader : sp<Builder<Shader>>::adopt(new GLShaderBuilderImpl(factory, doc, resourceLoaderContext, stringTable->getString(defVertex), stringTable->getString(defFragment)));
}

sp<Shader> Shader::fromStringTable(const String& vertex, const String& fragment, const sp<Snippet>& snippet, const sp<ResourceLoaderContext>& resourceLoaderContext)
{
    const Global<StringTable> stringTable;
    const sp<PipelineBuildingContext> buildingContext = sp<PipelineBuildingContext>::make(resourceLoaderContext->renderController()->createPipelineFactory(), stringTable->getString(vertex), stringTable->getString(fragment));
    if(snippet)
        buildingContext->addSnippet(snippet);

    const sp<PipelineLayout> pipelineLayout = sp<PipelineLayout>::make(resourceLoaderContext->renderController(), buildingContext);
    return sp<Shader>::make(buildingContext->_shader, pipelineLayout, nullptr);
}

std::vector<Layer::UBOSnapshot> Shader::snapshot(MemoryPool& memoryPool) const
{
    std::vector<Layer::UBOSnapshot> uboSnapshot;
    for(const sp<PipelineInput::UBO>& i : _input->ubos())
        uboSnapshot.push_back(i->snapshot(memoryPool));
    return uboSnapshot;
}

void Shader::active(GraphicsContext& graphicsContext, const DrawingContext& drawingContext)
{
    buildPipeline(graphicsContext, drawingContext._shader_bindings)->active(graphicsContext, drawingContext);
}

const sp<PipelineInput>& Shader::input() const
{
    return _input;
}

const sp<Camera>& Shader::camera() const
{
    return _camera;
}

const sp<Pipeline>& Shader::pipeline() const
{
    return _stub->_pipeline;
}

const sp<PipelineFactory>& Shader::pipelineFactory() const
{
    return _stub->_pipeline_factory;
}

const sp<PipelineLayout>& Shader::pipelineLayout() const
{
    return _pipeline_layout;
}

const sp<Pipeline> Shader::buildPipeline(GraphicsContext& graphicsContext, const sp<ShaderBindings>& bindings) const
{
    return _stub->buildPipeline(graphicsContext, bindings);
}

uint32_t Shader::stride() const
{
    return _input->getStream(0).stride();
}

Shader::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _factory(factory), _manifest(manifest), _resource_loader_context(resourceLoaderContext), _vertex(Strings::load(manifest, "vertex", "@shaders:default.vert")),
      _fragment(Strings::load(manifest, "fragment", "@shaders:texture.frag")),
      _snippet(factory.getBuilder<Snippet>(manifest, Constants::Attributes::SNIPPET)),
      _camera(factory.getBuilder<Camera>(manifest, Constants::Attributes::CAMERA))
{
}

sp<Shader> Shader::BUILDER::build(const sp<Scope>& args)
{
    const sp<PipelineBuildingContext> buildingContext = sp<PipelineBuildingContext>::make(_resource_loader_context->renderController()->createPipelineFactory(), _vertex->build(args), _fragment->build(args));
    buildingContext->loadPredefinedParam(_factory, args, _manifest);
    if(_snippet)
        buildingContext->addSnippet(_snippet->build(args));

    const sp<PipelineLayout> pipelineLayout = sp<PipelineLayout>::make(_resource_loader_context->renderController(), buildingContext);
    return sp<Shader>::make(buildingContext->_shader, pipelineLayout, _camera->build(args));
}

Shader::Stub::Stub(const sp<PipelineFactory>& pipelineFactory)
    : _pipeline_factory(pipelineFactory)
{
}

sp<Pipeline> Shader::Stub::buildPipeline(GraphicsContext& graphicsContext, const sp<ShaderBindings>& shaderBindings)
{
    if(_pipeline)
    {
        if(_pipeline->id() == 0)
            _pipeline->upload(graphicsContext, nullptr);
        return _pipeline;
    }

    shaderBindings->pipelineLayout()->preCompile(graphicsContext, shaderBindings);
    _pipeline = _pipeline_factory->buildPipeline(graphicsContext, shaderBindings);
    graphicsContext.resourceManager()->upload(_pipeline, nullptr, ResourceManager::US_ON_SURFACE_READY);
    _pipeline->upload(graphicsContext, nullptr);
    return _pipeline;
}

}
