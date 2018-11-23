#include "renderer/base/gl_pipeline.h"

#include <set>
#include <regex>

#include "core/base/string_buffer.h"
#include "core/base/string_table.h"
#include "core/inf/array.h"
#include "core/impl/flatable/flatable_numeric.h"
#include "core/types/global.h"
#include "core/util/documents.h"
#include "core/util/strings.h"

#include "graphics/base/camera.h"
#include "graphics/base/color.h"
#include "renderer/base/varyings.h"
#include "graphics/impl/flatable/flatable_color3b.h"

#include "renderer/base/gl_program.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/inf/renderer_factory.h"
#include "renderer/opengl/util/gl_debug.h"

#include "platform/platform.h"

namespace ark {

namespace {

class GLShaderBuilderImpl : public Builder<GLPipeline> {
public:
    GLShaderBuilderImpl(BeanFactory& factory, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext, const String& vertexSrc, const String& fragmentSrc)
        : _factory(factory), _manifest(doc), _render_controller(resourceLoaderContext->renderController()), _vertex_src(vertexSrc), _fragment_src(fragmentSrc),
          _camera(factory.getBuilder<Camera>(doc, Constants::Attributes::CAMERA)) {
    }

    virtual sp<GLPipeline> build(const sp<Scope>& args) override {
        const sp<PipelineLayout> source = sp<PipelineLayout>::make(_render_controller, _vertex_src, _fragment_src);
        source->loadPredefinedParam(_factory, args, _manifest);
        return sp<GLPipeline>::make(source, _camera->build(args));
    }

private:
    BeanFactory _factory;
    document _manifest;
    sp<RenderController> _render_controller;

    String _vertex_src, _fragment_src;

    sp<Builder<Camera>> _camera;
};

}

GLPipeline::GLPipeline(const sp<PipelineLayout>& source, const sp<Camera>& camera)
    : _stub(sp<Stub>::make(source)), _camera(camera ? camera : Camera::getMainCamera())
{
    source->initialize();
}

sp<Builder<GLPipeline>> GLPipeline::fromDocument(BeanFactory& factory, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext, const String& defVertex, const String& defFragment)
{
    const Global<StringTable> stringTable;
    const sp<Builder<GLPipeline>> shader = factory.getBuilder<GLPipeline>(doc, Constants::Attributes::SHADER, false);
    return shader ? shader : sp<Builder<GLPipeline>>::adopt(new GLShaderBuilderImpl(factory, doc, resourceLoaderContext, stringTable->getString(defVertex), stringTable->getString(defFragment)));
}

sp<GLPipeline> GLPipeline::fromStringTable(const String& vertex, const String& fragment, const sp<GLSnippet>& snippet, const sp<ResourceLoaderContext>& resourceLoaderContext)
{
    const Global<StringTable> stringTable;
    const sp<PipelineLayout> source = sp<PipelineLayout>::make(resourceLoaderContext->renderController(), stringTable->getString(vertex), stringTable->getString(fragment));
    if(snippet)
        source->addSnippet(snippet);
    return sp<GLPipeline>::make(source, nullptr);
}

void GLPipeline::use(GraphicsContext& graphicsContext)
{
    getGLProgram(graphicsContext)->use();
}

void GLPipeline::bindUniforms(GraphicsContext& graphicsContext) const
{
    for(const GLUniform& uniform : _stub->_input->uniforms())
        uniform.prepare(graphicsContext, _stub->_program);
}

const sp<PipelineInput>& GLPipeline::input() const
{
    return _stub->_input;
}

const sp<Camera>& GLPipeline::camera() const
{
    return _camera;
}

const sp<GLProgram>& GLPipeline::program() const
{
    return _stub->_program;
}

const sp<GLSnippet>& GLPipeline::snippet() const
{
    return _stub->_snippet;
}

void GLPipeline::glUpdateMVPMatrix(GraphicsContext& graphicsContext, const Matrix& matrix) const
{
    glUpdateMatrix(graphicsContext, "u_MVP", matrix);
}

void GLPipeline::glUpdateVPMatrix(GraphicsContext& graphicsContext, const Matrix& matrix) const
{
    glUpdateMatrix(graphicsContext, "u_VP", matrix);
}

void GLPipeline::glUpdateMatrix(GraphicsContext& graphicsContext, const String& name, const Matrix& matrix) const
{
    const GLProgram::Uniform& uniform = _stub->_program->getUniform(name);
    DCHECK(uniform, "Uniform %s not found", name.c_str());
    uniform.setUniformMatrix4fv(1, GL_FALSE, matrix.value(), graphicsContext.tick());
}

const sp<GLProgram>& GLPipeline::getGLProgram(GraphicsContext& graphicsContext)
{
    if(_stub->_program)
    {
        if(_stub->id() == 0)
            _stub->prepare(graphicsContext);
        return _stub->_program;
    }

    _stub->_pipeline_factory->preprocess(graphicsContext);
    _stub->_program = _stub->_pipeline_factory->makeGLProgram(graphicsContext);
    graphicsContext.glResourceManager()->prepare(_stub, GLResourceManager::PS_ON_SURFACE_READY);
    _stub->prepare(graphicsContext);
    return _stub->_program;
}

uint32_t GLPipeline::stride() const
{
    return _stub->_input->getStream(0).stride();
}

GLPipeline::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _factory(factory), _manifest(manifest), _resource_loader_context(resourceLoaderContext), _vertex(Strings::load(manifest, "vertex", "@shaders:default.vert")),
      _fragment(Strings::load(manifest, "fragment", "@shaders:texture.frag")),
      _snippet(factory.getBuilder<GLSnippet>(manifest, Constants::Attributes::SNIPPET, false)),
      _camera(factory.getBuilder<Camera>(manifest, Constants::Attributes::CAMERA))
{
}

sp<GLPipeline> GLPipeline::BUILDER::build(const sp<Scope>& args)
{
    const sp<PipelineLayout> source = sp<PipelineLayout>::make(_resource_loader_context->renderController(), _vertex->build(args), _fragment->build(args));
    source->loadPredefinedParam(_factory, args, _manifest);
    if(_snippet)
        source->addSnippet(_snippet->build(args));
    return sp<GLPipeline>::make(source, _camera->build(args));
}

GLPipeline::Stub::Stub(const sp<PipelineLayout>& source)
    : _pipeline_factory(source), _input(source->input()), _snippet(source->snippet())
{
}

uint32_t GLPipeline::Stub::id()
{
    DASSERT(_program);
    return _program->id();
}

void GLPipeline::Stub::prepare(GraphicsContext& graphicsContext)
{
    DASSERT(_program);
    for(const GLUniform& i : _input->uniforms())
        i.notify();

    _program->prepare(graphicsContext);
}

void GLPipeline::Stub::recycle(GraphicsContext& graphicsContext)
{
    DASSERT(_program);
    _program->recycle(graphicsContext);
}

}
