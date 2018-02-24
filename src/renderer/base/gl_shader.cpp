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

#include "graphics/base/color.h"
#include "renderer/base/varyings.h"
#include "graphics/impl/flatable/flatable_color3b.h"

#include "renderer/base/gl_shader_source.h"
#include "renderer/base/gl_program.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/util/gl_debug.h"

#include "platform/platform.h"

namespace ark {

namespace {

class GLShaderBuilderImpl : public Builder<GLShader> {
public:
    GLShaderBuilderImpl(BeanFactory& factory, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext, const String& vertexSrc, const String& fragmentSrc)
        : _factory(factory), _manifest(doc), _resource_loader_context(resourceLoaderContext), _vertex_src(vertexSrc), _fragment_src(fragmentSrc) {
    }

    virtual sp<GLShader> build(const sp<Scope>& args) override {
        const sp<GLShaderSource> source = sp<GLShaderSource>::make(_vertex_src, _fragment_src, _resource_loader_context->renderController());
        source->loadPredefinedParam(_factory, args, _manifest);
        return sp<GLShader>::make(source);
    }

private:
    BeanFactory _factory;
    document _manifest;
    sp<ResourceLoaderContext> _resource_loader_context;

    String _vertex_src, _fragment_src;
};

}

GLShader::GLShader(const sp<GLShaderSource>& source)
    : _source(source)
{
    _source->initialize();
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
    return sp<GLShader>::make(source);
}

void GLShader::use(GraphicsContext& graphicsContext)
{
    graphicsContext.glUseProgram(graphicsContext.getGLProgram(*this));
}

GLShader::Slot GLShader::preprocess(GraphicsContext& graphicsContext)
{
    return _source->preprocess(graphicsContext);
}

void GLShader::bindUniforms(GraphicsContext& graphicsContext) const
{
    for(const GLUniform& uniform : _source->_uniforms)
        uniform.prepare(graphicsContext, _program);
}

void GLShader::bindAttributes(GraphicsContext& graphicsContext) const
{
    bindAttributes(graphicsContext, _program);
}

void GLShader::bindAttributes(GraphicsContext& /*graphicsContext*/, const sp<GLProgram>& program) const
{
    DCHECK(program && program->id(), "GLProgram unprepared");
    for(const auto& i : _source->_attributes)
    {
        const GLAttribute& attr = i.second;
        const GLProgram::Attribute& glAttribute = program->getAttribute(attr.name());
        attr.setVertexPointer(glAttribute.location(), _source->_stride);
    }
}

const sp<GLShaderSource>& GLShader::source() const
{
    return _source;
}

const sp<GLProgram>& GLShader::program() const
{
    return _program;
}

void GLShader::setProgram(const sp<GLProgram>& program)
{
    _program = program;
}

const sp<GLSnippet>& GLShader::snippet() const
{
    return _source->snippet();
}

const sp<GLProgram>& GLShader::makeGLProgram(GraphicsContext& graphicsContext)
{
    _program = _source->makeGLProgram(graphicsContext);
    return _program;
}

uint32_t GLShader::stride() const
{
    return _source->_stride;
}

const GLAttribute& GLShader::getAttribute(const String& name) const
{
    return _source->getAttribute(name);
}

sp<Varyings> GLShader::makeVaryings() const
{
    return sp<Varyings>::make(*this);
}

GLShader::Slot::Slot(const String& vertex, const String& fragment)
    : _vertex_shader_hash(Strings::hash(vertex)), _fragment_shader_hash(Strings::hash(fragment))
{
}

bool GLShader::Slot::operator <(const GLShader::Slot& other) const
{
    return _vertex_shader_hash < other._vertex_shader_hash || (_vertex_shader_hash == other._vertex_shader_hash && _fragment_shader_hash < other._fragment_shader_hash);
}

GLShader::BUILDER::BUILDER(BeanFactory& parent, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _factory(parent), _manifest(doc), _resource_loader_context(resourceLoaderContext), _vertex(Strings::load(doc, "vertex", "@shaders:texture.vert")),
      _fragment(Strings::load(doc, "fragment", "@shaders:texture.frag")),
      _snippet(parent.getBuilder<GLSnippet>(doc, Constants::Attributes::SNIPPET, false))
{
}

sp<GLShader> GLShader::BUILDER::build(const sp<Scope>& args)
{
    const sp<GLShaderSource> source = sp<GLShaderSource>::make(_vertex->build(args), _fragment->build(args), _resource_loader_context->renderController());
    source->loadPredefinedParam(_factory, args, _manifest);
    if(_snippet)
        source->addSnippet(_snippet->build(args));
    return sp<GLShader>::make(source);
}

}
