#include "renderer/base/gl_shader.h"

#include <set>
#include <regex>

#include "core/base/string_builder.h"
#include "core/base/string_table.h"
#include "core/inf/array.h"
#include "core/impl/flatable/flatable_numeric.h"
#include "core/types/global.h"
#include "core/util/documents.h"
#include "core/util/strings.h"

#include "graphics/base/color.h"
#include "graphics/base/filter.h"
#include "graphics/impl/flatable/flatable_color3b.h"

#include "renderer/base/gl_shader_source.h"
#include "renderer/base/gl_program.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/gles20/impl/gl_snippet/bind_attributes.h"

#include "platform/platform.h"

namespace ark {

namespace {

class GLShaderBuilderImpl : public Builder<GLShader> {
public:
    GLShaderBuilderImpl(BeanFactory& factory, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext, const String& vertexSrc, const String& fragmentSrc)
        : _factory(factory), _manifest(doc), _resource_loader_context(resourceLoaderContext), _vertex_src(vertexSrc), _fragment_src(fragmentSrc) {
    }

    virtual sp<GLShader> build(const sp<Scope>& args) override {
        const sp<GLShaderSource> source = sp<GLShaderSource>::make(_vertex_src, _fragment_src, _resource_loader_context->synchronizer());
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
    : _source(source), _slot(_source->preprocess())
{
}

GLShader::GLShader(const GLShader& other)
    : _source(other._source), _slot(other._slot), _program(other._program)
{
}

GLShader::GLShader(GLShader&& other)
    : _source(std::move(other._source)), _slot(other._slot), _program(std::move(other._program))
{
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
    const sp<GLShaderSource> source = sp<GLShaderSource>::make(stringTable->getString(vertex), stringTable->getString(fragment), resourceLoaderContext->synchronizer());
    if(snippet)
        source->addSnippet(snippet);
    return sp<GLShader>::make(source);
}

void GLShader::use(GraphicsContext& graphicsContext)
{
    graphicsContext.glUseProgram(graphicsContext.getGLProgram(*this));
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

const sp<GLProgram>& GLShader::program() const
{
    return _program;
}

void GLShader::setProgram(const sp<GLProgram>& program)
{
    _program = program;
}

const GLShader::Slot& GLShader::slot() const
{
    return _slot;
}

const sp<GLSnippet>& GLShader::snippet() const
{
    return _source->snippet();
}

const sp<GLProgram>& GLShader::makeGLProgram()
{
    _program = _source->makeGLProgram();
    return _program;
}

uint32_t GLShader::stride() const
{
    return _source->_stride;
}

const GLAttribute& GLShader::getAttribute(const String& name) const
{
    auto iter = _source->_attributes.find(name);
    DCHECK(iter != _source->_attributes.end(), "Shader has no attribute \"%s\"", name.c_str());
    return iter->second;
}

GLShader::Slot::Slot(const String& vertex, const String& fragment)
    : _vertex_shader_hash(Strings::hash(vertex)), _fragment_shader_hash(Strings::hash(fragment))
{
}

GLShader::Slot::Slot(const GLShader::Slot& other)
    : _vertex_shader_hash(other._vertex_shader_hash), _fragment_shader_hash(other._fragment_shader_hash)
{
}

bool GLShader::Slot::operator <(const GLShader::Slot& other) const
{
    return _vertex_shader_hash < other._vertex_shader_hash || (_vertex_shader_hash == other._vertex_shader_hash && _fragment_shader_hash < other._fragment_shader_hash);
}

GLShader::BUILDER::BUILDER(BeanFactory& parent, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _factory(parent), _manifest(doc), _resource_loader_context(resourceLoaderContext), _vertex(Strings::load(doc, "vertex-shader", "@shaders:texture.vert")),
      _fragment(Strings::load(doc, "fragment-shader", "@shaders:texture.frag")),
      _snippet(parent.getBuilder<GLSnippet>(doc, Constants::Attributes::SNIPPET, false))
{
}

sp<GLShader> GLShader::BUILDER::build(const sp<Scope>& args)
{
    const sp<GLShaderSource> source = sp<GLShaderSource>::make(_vertex->build(args), _fragment->build(args), _resource_loader_context->synchronizer());
    source->loadPredefinedParam(_factory, args, _manifest);
    if(_snippet)
        source->addSnippet(_snippet->build(args));
    return sp<GLShader>::make(source);
}

}
