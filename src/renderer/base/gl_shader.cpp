#include "renderer/base/gl_shader.h"

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
#include "renderer/base/gl_shader_source.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/util/gl_debug.h"

#include "platform/platform.h"

namespace ark {

namespace {

class GLShaderBuilderImpl : public Builder<GLShader> {
public:
    GLShaderBuilderImpl(BeanFactory& factory, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext, const String& vertexSrc, const String& fragmentSrc)
        : _factory(factory), _manifest(doc), _resource_loader_context(resourceLoaderContext), _vertex_src(vertexSrc), _fragment_src(fragmentSrc),
          _camera(factory.getBuilder<Camera>(doc, Constants::Attributes::CAMERA)) {
    }

    virtual sp<GLShader> build(const sp<Scope>& args) override {
        const sp<GLShaderSource> source = sp<GLShaderSource>::make(_vertex_src, _fragment_src, _resource_loader_context->renderController());
        source->loadPredefinedParam(_factory, args, _manifest);
        return sp<GLShader>::make(source, _camera->build(args));
    }

private:
    BeanFactory _factory;
    document _manifest;
    sp<ResourceLoaderContext> _resource_loader_context;

    String _vertex_src, _fragment_src;

    sp<Builder<Camera>> _camera;
};

}

GLShader::GLShader(const sp<GLShaderSource>& source, const sp<Camera>& camera)
    : _stub(sp<Stub>::make(source)), _camera(camera ? camera : Camera::getMainCamera())
{
    _stub->_source->initialize();
}

sp<Builder<GLShader>> GLShader::fromDocument(BeanFactory& factory, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext, const String& defVertex, const String& defFragment)
{
    const Global<StringTable> stringTable;
    const sp<Builder<GLShader>> shader = factory.getBuilder<GLShader>(doc, Constants::Attributes::SHADER, false);
    return shader ? shader : sp<Builder<GLShader>>::adopt(new GLShaderBuilderImpl(factory, doc, resourceLoaderContext, stringTable->getString(defVertex), stringTable->getString(defFragment)));
}

sp<GLShader> GLShader::fromStringTable(const String& vertex, const String& fragment, const sp<GLSnippet>& snippet, const sp<ResourceLoaderContext>& resourceLoaderContext)
{
    const Global<StringTable> stringTable;
    const sp<GLShaderSource> source = sp<GLShaderSource>::make(stringTable->getString(vertex), stringTable->getString(fragment), resourceLoaderContext->renderController());
    if(snippet)
        source->addSnippet(snippet);
    return sp<GLShader>::make(source, nullptr);
}

void GLShader::use(GraphicsContext& graphicsContext)
{
    getGLProgram(graphicsContext)->use();
}

void GLShader::bindUniforms(GraphicsContext& graphicsContext) const
{
    for(const GLUniform& uniform : _stub->_source->_uniforms)
        uniform.prepare(graphicsContext, _stub->_program);
}

const sp<GLShaderSource>& GLShader::source() const
{
    return _stub->_source;
}

const sp<Camera>& GLShader::camera() const
{
    return _camera;
}

const sp<GLProgram>& GLShader::program() const
{
    return _stub->_program;
}

const sp<GLSnippet>& GLShader::snippet() const
{
    return _stub->_source->snippet();
}

void GLShader::glUpdateMVPMatrix(GraphicsContext& graphicsContext, const Matrix& matrix) const
{
    glUpdateMatrix(graphicsContext, "u_MVP", matrix);
}

void GLShader::glUpdateVPMatrix(GraphicsContext& graphicsContext, const Matrix& matrix) const
{
    glUpdateMatrix(graphicsContext, "u_VP", matrix);
}

void GLShader::glUpdateMatrix(GraphicsContext& graphicsContext, const String& name, const Matrix& matrix) const
{
    const GLProgram::Uniform& uniform = _stub->_program->getUniform(name);
    DCHECK(uniform, "Uniform %s not found", name.c_str());
    uniform.setUniformMatrix4fv(1, GL_FALSE, matrix.value(), graphicsContext.tick());
}

const sp<GLProgram>& GLShader::getGLProgram(GraphicsContext& graphicsContext)
{
    if(_stub->_program)
    {
        if(_stub->id() == 0)
            _stub->prepare(graphicsContext);
        return _stub->_program;
    }

    _stub->_source->preprocess(graphicsContext);
    _stub->_program = _stub->_source->makeGLProgram(graphicsContext);
    graphicsContext.glResourceManager()->prepare(_stub, GLResourceManager::PS_ON_SURFACE_READY);
    _stub->prepare(graphicsContext);
    return _stub->_program;
}

uint32_t GLShader::stride() const
{
    return _stub->_source->_input->getStream(0).stride();
}

const GLAttribute& GLShader::getAttribute(const String& name, uint32_t divisor) const
{
    return _stub->_source->input()->getAttribute(name, divisor);
}

GLShader::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _factory(factory), _manifest(manifest), _resource_loader_context(resourceLoaderContext), _vertex(Strings::load(manifest, "vertex", "@shaders:default.vert")),
      _fragment(Strings::load(manifest, "fragment", "@shaders:texture.frag")),
      _snippet(factory.getBuilder<GLSnippet>(manifest, Constants::Attributes::SNIPPET, false)),
      _camera(factory.getBuilder<Camera>(manifest, Constants::Attributes::CAMERA))
{
}

sp<GLShader> GLShader::BUILDER::build(const sp<Scope>& args)
{
    const sp<GLShaderSource> source = sp<GLShaderSource>::make(_vertex->build(args), _fragment->build(args), _resource_loader_context->renderController());
    source->loadPredefinedParam(_factory, args, _manifest);
    if(_snippet)
        source->addSnippet(_snippet->build(args));
    return sp<GLShader>::make(source, _camera->build(args));
}

GLShader::Stub::Stub(const sp<GLShaderSource>& source)
    : _source(source)
{
}

uint32_t GLShader::Stub::id()
{
    DASSERT(_program);
    return _program->id();
}

void GLShader::Stub::prepare(GraphicsContext& graphicsContext)
{
    DASSERT(_program);
    for(const GLUniform& i : _source->uniforms())
        i.notify();

    _program->prepare(graphicsContext);
}

void GLShader::Stub::recycle(GraphicsContext& graphicsContext)
{
    DASSERT(_program);
    _program->recycle(graphicsContext);
}

}
