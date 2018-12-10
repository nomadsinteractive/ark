#include "renderer/base/shader.h"

#include <set>
#include <regex>

#include "core/base/string_buffer.h"
#include "core/base/string_table.h"
#include "core/inf/array.h"
#include "core/impl/flatable/flatable_numeric.h"
#include "core/types/safe_ptr.h"
#include "core/types/global.h"
#include "core/util/documents.h"
#include "core/util/strings.h"

#include "graphics/base/camera.h"
#include "graphics/base/color.h"
#include "renderer/base/varyings.h"
#include "graphics/impl/flatable/flatable_color3b.h"

#include "renderer/base/resource_manager.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/inf/renderer_factory.h"
#include "renderer/inf/pipeline_factory.h"

#include "renderer/opengl/base/gl_pipeline.h"
#include "renderer/opengl/util/gl_debug.h"

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
        const sp<PipelineLayout> source = sp<PipelineLayout>::make(_render_controller, _vertex_src, _fragment_src);
        source->loadPredefinedParam(_factory, args, _manifest);
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

Shader::Shader(const sp<PipelineLayout>& source, const sp<Camera>& camera)
    : _stub(sp<Stub>::make(source)), _camera(camera ? camera : Camera::getMainCamera())
{
    source->initialize();
}

sp<Builder<Shader>> Shader::fromDocument(BeanFactory& factory, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext, const String& defVertex, const String& defFragment)
{
    const Global<StringTable> stringTable;
    const sp<Builder<Shader>> shader = factory.getBuilder<Shader>(doc, Constants::Attributes::SHADER);
    return shader ? shader : sp<Builder<Shader>>::adopt(new GLShaderBuilderImpl(factory, doc, resourceLoaderContext, stringTable->getString(defVertex), stringTable->getString(defFragment)));
}

sp<Shader> Shader::fromStringTable(const String& vertex, const String& fragment, const sp<GLSnippet>& snippet, const sp<ResourceLoaderContext>& resourceLoaderContext)
{
    const Global<StringTable> stringTable;
    const sp<PipelineLayout> source = sp<PipelineLayout>::make(resourceLoaderContext->renderController(), stringTable->getString(vertex), stringTable->getString(fragment));
    if(snippet)
        source->addSnippet(snippet);
    return sp<Shader>::make(source, nullptr);
}

void Shader::use(GraphicsContext& graphicsContext)
{
    getPipeline(graphicsContext)->use();
}

void Shader::bindUniforms(GraphicsContext& graphicsContext) const
{
    for(const Uniform& uniform : _stub->_input->uniforms())
        uniform.prepare(graphicsContext, _stub->_pipeline);
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

const sp<GLSnippet>& Shader::snippet() const
{
    return _stub->_snippet;
}

//void Shader::glUpdateMVPMatrix(GraphicsContext& graphicsContext, const Matrix& matrix) const
//{
//    glUpdateMatrix(graphicsContext, "u_MVP", matrix);
//}

//void Shader::glUpdateVPMatrix(GraphicsContext& graphicsContext, const Matrix& matrix) const
//{
//    glUpdateMatrix(graphicsContext, "u_VP", matrix);
//}

//void Shader::glUpdateMatrix(GraphicsContext& graphicsContext, const String& name, const Matrix& matrix) const
//{
//    const sp<GLPipeline> glPipeline = _stub->_pipeline;
//    const GLPipeline::GLUniform& uniform = glPipeline->getUniform(name);
//    DCHECK(uniform, "Uniform %s not found", name.c_str());
//    uniform.setUniformMatrix4fv(1, GL_FALSE, matrix.value(), graphicsContext.tick());
//}

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
      _snippet(factory.getBuilder<GLSnippet>(manifest, Constants::Attributes::SNIPPET)),
      _camera(factory.getBuilder<Camera>(manifest, Constants::Attributes::CAMERA))
{
}

sp<Shader> Shader::BUILDER::build(const sp<Scope>& args)
{
    const sp<PipelineLayout> source = sp<PipelineLayout>::make(_resource_loader_context->renderController(), _vertex->build(args), _fragment->build(args));
    source->loadPredefinedParam(_factory, args, _manifest);
    if(_snippet)
        source->addSnippet(_snippet->build(args));
    return sp<Shader>::make(source, _camera->build(args));
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
