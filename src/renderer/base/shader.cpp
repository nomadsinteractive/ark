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
#include "renderer/base/ubo.h"
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
        const sp<PipelineBuildingContext> buildingContext = sp<PipelineBuildingContext>::make(_vertex_src, _fragment_src);
        buildingContext->loadPredefinedParam(_factory, args, _manifest);

        const sp<PipelineLayout> source = sp<PipelineLayout>::make(_render_controller, buildingContext);
        return sp<Shader>::make(source, _camera->build(args));
    }

private:
    BeanFactory _factory;
    document _manifest;
    sp<RenderController> _render_controller;

    String _vertex_src, _fragment_src;

    SafePtr<Builder<Camera>> _camera;
};

}

Shader::Shader(const sp<PipelineLayout>& pipelineLayout, const sp<Camera>& camera)
    : _stub(sp<Stub>::make(pipelineLayout)), _camera(camera ? camera : Camera::getMainCamera())
{
    pipelineLayout->initialize(_camera);
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
    const sp<PipelineBuildingContext> buildingContext = sp<PipelineBuildingContext>::make(stringTable->getString(vertex), stringTable->getString(fragment));
    if(snippet)
        buildingContext->addSnippet(snippet);

    const sp<PipelineLayout> pipelineLayout = sp<PipelineLayout>::make(resourceLoaderContext->renderController(), buildingContext);
    return sp<Shader>::make(pipelineLayout, nullptr);
}

Layer::UBOSnapshot Shader::snapshot(MemoryPool& memoryPool) const
{
    return _stub->_input->ubo()->snapshot(memoryPool);
}

void Shader::active(GraphicsContext& graphicsContext, const DrawingContext& drawingContext)
{
    getPipeline(graphicsContext)->active(graphicsContext, drawingContext);
}

const sp<PipelineInput>& Shader::input() const
{
    return _stub->_input;
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

const sp<Snippet>& Shader::snippet() const
{
    return _stub->_snippet;
}

const sp<Pipeline>& Shader::getPipeline(GraphicsContext& graphicsContext)
{
    if(_stub->_pipeline)
    {
        if(_stub->id() == 0)
            _stub->upload(graphicsContext);
        return _stub->_pipeline;
    }

    _stub->_pipeline_layout->preCompile(graphicsContext);
    _stub->_pipeline = _stub->_pipeline_factory->buildPipeline(graphicsContext, _stub->_pipeline_layout);
    graphicsContext.resourceManager()->upload(_stub, ResourceManager::US_ON_SURFACE_READY);
    _stub->upload(graphicsContext);
    return _stub->_pipeline;
}

uint32_t Shader::stride() const
{
    return _stub->_input->getStream(0).stride();
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
    const sp<PipelineBuildingContext> buildingContext = sp<PipelineBuildingContext>::make(_vertex->build(args), _fragment->build(args));
    buildingContext->loadPredefinedParam(_factory, args, _manifest);
    if(_snippet)
        buildingContext->addSnippet(_snippet->build(args));

    const sp<PipelineLayout> pipelineLayout = sp<PipelineLayout>::make(_resource_loader_context->renderController(), buildingContext);
    return sp<Shader>::make(pipelineLayout, _camera->build(args));
}

Shader::Stub::Stub(const sp<PipelineLayout>& pipelineLayout)
    : _pipeline_factory(pipelineLayout->renderController()->createPipelineFactory()), _pipeline_layout(pipelineLayout), _input(pipelineLayout->input()), _snippet(pipelineLayout->snippet())
{
}

uint32_t Shader::Stub::id()
{
    DASSERT(_pipeline);
    return _pipeline->id();
}

void Shader::Stub::upload(GraphicsContext& graphicsContext)
{
    DASSERT(_pipeline);
    for(const Uniform& i : _input->uniforms())
        i.notify();

    _pipeline->upload(graphicsContext);
}

Resource::RecycleFunc Shader::Stub::recycle()
{
    DASSERT(_pipeline);
    return _pipeline->recycle();
}

}
