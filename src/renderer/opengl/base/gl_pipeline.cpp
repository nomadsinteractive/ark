#include "renderer/opengl/base/gl_pipeline.h"

#include "core/inf/flatable.h"
#include "core/types/weak_ptr.h"
#include "core/util/log.h"
#include "core/util/strings.h"

#include "graphics/base/color.h"
#include "graphics/base/matrix.h"

#include "renderer/base/recycler.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/texture.h"
#include "renderer/base/uniform.h"
#include "renderer/inf/resource.h"
#include "renderer/util/vertex_util.h"

#include "renderer/opengl/util/gl_util.h"

#include "platform/platform.h"

namespace ark {
namespace opengl {

namespace {

class GLScissor {
public:
    GLScissor(const Rect& rect, bool enabled)
        : _enabled(enabled) {
        if(_enabled) {
            glEnable(GL_SCISSOR_TEST);
            glScissor(static_cast<GLint>(rect.left()), static_cast<GLint>(rect.top()), static_cast<GLsizei>(rect.width()), static_cast<GLsizei>(rect.height()));
        }
    }
    ~GLScissor() {
        if(_enabled)
            glDisable(GL_SCISSOR_TEST);
    }

private:
    bool _enabled;
};

class GLCullFace {
public:
    GLCullFace(bool enabled)
        : _enabled(enabled) {
        if(_enabled)
            glEnable(GL_CULL_FACE);
    }
    ~GLCullFace() {
        if(_enabled)
            glDisable(GL_CULL_FACE);
    }

private:
    bool _enabled;
};


}

GLPipeline::GLPipeline(const sp<Recycler>& recycler, uint32_t version, const String& vertexShader, const String& fragmentShader, const PipelineBindings& bindings)
    : _recycler(recycler), _pipeline_input(bindings.input()), _version(version), _vertex_source(vertexShader), _fragment_source(fragmentShader), _cull_face(bindings.getFlag(PipelineBindings::FLAG_CULL_MODE_BITMASK) != PipelineBindings::FLAG_CULL_MODE_NONE),
      _scissor(bindings.scissor()), _scissor_enabled(VertexUtil::isScissorEnabled(_scissor)), _id(0), _render_command(createRenderCommand(bindings)), _rebind_needed(true)
{
}

GLPipeline::~GLPipeline()
{
    if(_id)
        _recycler->recycle(*this);
}

uint64_t GLPipeline::id()
{
    return _id;
}

void GLPipeline::upload(GraphicsContext& graphicsContext, const sp<Uploader>& /*uploader*/)
{
    for(const sp<PipelineInput::UBO>& i : _pipeline_input->ubos())
        i->notify();

    _vertex_shader = makeShader(graphicsContext, _version, GL_VERTEX_SHADER, _vertex_source);
    _fragment_shader = makeShader(graphicsContext, _version, GL_FRAGMENT_SHADER, _fragment_source);
    _id = glCreateProgram();
    glAttachShader(_id, _vertex_shader->id());
    glAttachShader(_id, _fragment_shader->id());
    glLinkProgram(_id);
    glDetachShader(_id, _vertex_shader->id());
    glDetachShader(_id, _fragment_shader->id());

    GLint linkstatus = 0;
    glGetProgramiv(_id, GL_LINK_STATUS, &linkstatus);
    DCHECK(linkstatus, "Program link failed: %s", getInformationLog().c_str());
    LOGD("GLProgram[%d]: vertex-shader: %d, fragment-shader: %d", _id, _vertex_shader->id(), _fragment_shader->id());
}

Resource::RecycleFunc GLPipeline::recycle()
{
    uint32_t id = _id;
    _id = 0;

    _vertex_shader = nullptr;
    _fragment_shader = nullptr;

    _attributes.clear();
    _uniforms.clear();

    return [id](GraphicsContext&) {
        LOGD("glDeleteProgram(%d)", id);
        glDeleteProgram(id);
    };
}

void GLPipeline::bindUBO(const RenderLayer::UBOSnapshot& uboSnapshot, const sp<PipelineInput::UBO>& ubo)
{
    const std::vector<sp<Uniform>>& uniforms = ubo->uniforms().values();
    for(size_t i = 0; i < uniforms.size(); ++i)
    {
        if(uboSnapshot._dirty_flags.buf()[i] || _rebind_needed)
        {
            const sp<Uniform>& uniform = uniforms.at(i);
            uint8_t* buf = uboSnapshot._buffer.buf();
            const auto pair = ubo->slots().at(i);
            bindUniform(reinterpret_cast<float*>(buf + pair.first), pair.second, uniform);
        }
    }
    _rebind_needed = false;
}

void GLPipeline::bind(GraphicsContext& /*graphicsContext*/, const DrawingContext& drawingContext)
{
    const std::vector<RenderLayer::UBOSnapshot>& uboSnapshots = drawingContext._ubos;

    const sp<PipelineInput>& pipelineInput = drawingContext._shader_bindings->pipelineInput();
    DCHECK(uboSnapshots.size() == pipelineInput->ubos().size(), "UBO Snapshot and UBO Layout mismatch: %d vs %d", uboSnapshots.size(), pipelineInput->ubos().size());

    glUseProgram(_id);
    for(size_t i = 0; i < uboSnapshots.size(); ++i)
    {
        const RenderLayer::UBOSnapshot& uboSnapshot = uboSnapshots.at(i);
        const sp<PipelineInput::UBO>& ubo = pipelineInput->ubos().at(i);
        bindUBO(uboSnapshot, ubo);
    }

    const std::vector<sp<Texture>>& samplers = drawingContext._shader_bindings->samplers();
    for(size_t i = 0; i < samplers.size(); ++i)
    {
        const sp<Texture>& sampler = samplers.at(i);
        DWARN(sampler, "Pipeline has unbound sampler at: %d", i);
        if(sampler)
            activeTexture(sampler, i);
    }
}

void GLPipeline::draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext)
{
    const GLCullFace cullFace(_cull_face);
    bool contextScissorEnabled = VertexUtil::isScissorEnabled(drawingContext._parameters._scissor);
    const GLScissor scissor(contextScissorEnabled ? drawingContext._parameters._scissor : _scissor, contextScissorEnabled || _scissor_enabled);

    _render_command->_parameters = drawingContext._parameters;
    _render_command->draw(graphicsContext);
}

void GLPipeline::bindBuffer(GraphicsContext& graphicsContext, const PipelineInput& input, const std::map<uint32_t, Buffer>& divisors)
{
    DCHECK(id(), "GLProgram unprepared");
    bindBuffer(graphicsContext, input, 0);
    for(const auto& i : divisors)
    {
        if(!i.second.id())
            i.second.upload(graphicsContext);

        glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(i.second.id()));
        bindBuffer(graphicsContext, input, i.first);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void GLPipeline::bindUniform(float* buf, uint32_t size, const Uniform& uniform)
{
    const GLPipeline::GLUniform& glUniform = getUniform(uniform.name());
    switch(uniform.type()) {
    case Uniform::TYPE_I1:
        DCHECK(size == 4, "Wrong uniform1i size: %d", size);
        glUniform.setUniform1i(*reinterpret_cast<int32_t*>(buf));
        break;
    case Uniform::TYPE_F1:
        DCHECK(size == 4, "Wrong uniform1f size: %d", size);
        glUniform.setUniform1f(buf[0]);
        break;
    case Uniform::TYPE_F2:
        DCHECK(size == 8, "Wrong uniform2f size: %d", size);
        glUniform.setUniform2f(buf[0], buf[1]);
        break;
    case Uniform::TYPE_F3:
        DCHECK(size >= 12, "Wrong uniform3f size: %d", size);
        glUniform.setUniform3f(buf[0], buf[1], buf[2]);
        break;
    case Uniform::TYPE_F4:
        DCHECK(size == 16, "Wrong uniform4f size: %d", size);
        glUniform.setUniform4f(buf[0], buf[1], buf[2], buf[3]);
        break;
    case Uniform::TYPE_F4V:
        DCHECK(size % 16 == 0, "Wrong uniform4fv size: %d", size);
        glUniform.setUniform4fv(size / 16, buf);
        break;
    case Uniform::TYPE_MAT4:
    case Uniform::TYPE_MAT4V:
        DCHECK(size % 64 == 0, "Wrong mat4fv size: %d", size);
        glUniform.setUniformMatrix4fv(size / 64, GL_FALSE, buf);
        break;
    default:
        DFATAL("Unimplemented");
    }
}

sp<GLPipeline::GLRenderCommand> GLPipeline::createRenderCommand(const PipelineBindings& bindings) const
{
    GLenum mode = GLUtil::toEnum(bindings.mode());
    if(bindings.hasDivisors())
        return sp<GLDrawElementsInstanced>::make(mode);
    return sp<GLDrawElements>::make(mode);
}

void GLPipeline::bindUniform(GraphicsContext& /*graphicsContext*/, const Uniform& uniform)
{
    float buf[1024];
    const sp<Flatable>& flatable = uniform.flatable();
    uint32_t size = flatable->size();
    DCHECK(size <= sizeof(buf), "Size too large: %d", size);
    flatable->flat(buf);
    bindUniform(buf, size, uniform);
}

void GLPipeline::activeTexture(const Texture& texture, uint32_t name)
{
    static GLenum glTargets[Texture::TYPE_COUNT] = {GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP};
    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + name));
    glBindTexture(glTargets[texture.type()], static_cast<GLuint>(texture.delegate()->id()));

    char uniformName[16] = {'u', '_', 'T', 'e', 'x', 't', 'u', 'r', 'e', static_cast<char>('0' + name)};
    const GLPipeline::GLUniform& uTexture = getUniform(uniformName);
    uTexture.setUniform1i(static_cast<GLint>(name));
}

void GLPipeline::glUpdateMatrix(GraphicsContext& /*graphicsContext*/, const String& name, const Matrix& matrix)
{
    const GLPipeline::GLUniform& uniform = getUniform(name);
    DCHECK(uniform, "Uniform %s not found", name.c_str());
    uniform.setUniformMatrix4fv(1, GL_FALSE, matrix.value());
}

void GLPipeline::bindBuffer(GraphicsContext& /*graphicsContext*/, const PipelineInput& input, uint32_t divisor)
{
    const PipelineInput::Stream& stream = input.getStream(divisor);
    for(const auto& i : stream.attributes().values())
    {
        const GLPipeline::GLAttribute& glAttribute = getAttribute(i.name());
        glAttribute.bind(i, stream.stride());
    }
}

GLint GLPipeline::getAttribLocation(const String& name)
{
    return glGetAttribLocation(_id, name.c_str());
}

GLint GLPipeline::getUniformLocation(const String& name)
{
    GLint location = glGetUniformLocation(_id, name.c_str());
    DWARN(location != -1, "Undefine uniform \"%s\". It might be optimized out, or something goes wrong.", name.c_str());
    return location;
}

const GLPipeline::GLAttribute& GLPipeline::getAttribute(const String& name)
{
    const auto iter = _attributes.find(name);
    if(iter != _attributes.end())
        return iter->second;
    _attributes[name] = getAttribLocation(name);
    return _attributes[name];
}

const GLPipeline::GLUniform& GLPipeline::getUniform(const String& name)
{
    const auto iter = _uniforms.find(name);
    if(iter != _uniforms.end())
        return iter->second;
    _uniforms[name] = getUniformLocation(name);
    return _uniforms[name];
}

String GLPipeline::getInformationLog() const
{
    GLint length = 0;
    glGetProgramiv(_id, GL_INFO_LOG_LENGTH, &length);

    size_t len = static_cast<size_t>(length);
    std::vector<GLchar> infos(len + 1);
    glGetProgramInfoLog(_id, length, &length, infos.data());
    infos.back() = 0;
    return infos.data();
}

sp<GLPipeline::Shader> GLPipeline::makeShader(GraphicsContext& graphicsContext, uint32_t version, GLenum type, const String& source) const
{
    typedef std::unordered_map<GLenum, std::map<String, WeakPtr<Shader>>> ShaderPool;

    ShaderPool& shaders = *graphicsContext.attachment<ShaderPool>();
    const auto iter = shaders[type].find(source);
    if(iter != shaders[type].end())
    {
        const sp<GLPipeline::Shader> shader = iter->second.lock();
        if(shader)
            return shader;
    }

    const sp<Shader> shader = sp<Shader>::make(graphicsContext.renderController()->recycler(), version, type, source);
    shaders[type][source] = shader;
    return shader;
}

GLPipeline::GLAttribute::GLAttribute(GLint location)
    : _location(location)
{
}

void GLPipeline::GLAttribute::bind(const Attribute& attribute, GLsizei stride) const
{
    DWARN(_location >= 0, "Attribute \"%s\" location: %d", attribute.name().c_str(), _location);
    if(attribute.length() <= 4)
        setVertexPointer(attribute, _location, stride, attribute.length(), attribute.offset());
    else if(attribute.length() == 16)
    {
        uint32_t offset = attribute.size() / 4;
        for(int32_t i = 0; i < 4; i++)
            setVertexPointer(attribute, _location + i, stride, 4, attribute.offset() + offset * i);
    }
    else if(attribute.length() == 9)
    {
        uint32_t offset = attribute.size() / 3;
        for(int32_t i = 0; i < 3; i++)
            setVertexPointer(attribute, _location + i, stride, 3, attribute.offset() + offset * i);
    }
}

void GLPipeline::GLAttribute::setVertexPointer(const Attribute& attribute, GLuint location, GLsizei stride, uint32_t length, uint32_t offset) const
{
    static const GLenum glTypes[Attribute::TYPE_COUNT] = {GL_BYTE, GL_FLOAT, GL_INT, GL_SHORT, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT};

    glEnableVertexAttribArray(location);
    if(attribute.type() == Attribute::TYPE_FLOAT || attribute.normalized())
        glVertexAttribPointer(location, length, glTypes[attribute.type()], attribute.normalized() ? GL_TRUE : GL_FALSE, stride, reinterpret_cast<void*>(offset));
    else
        glVertexAttribIPointer(location, length, glTypes[attribute.type()], stride, reinterpret_cast<void*>(offset));

    if(attribute.divisor())
        glVertexAttribDivisor(location, attribute.divisor());
}

GLPipeline::GLUniform::GLUniform(GLint location)
    : _location(location)
{
}

GLPipeline::GLUniform::operator bool() const
{
    return _location != -1;
}

void GLPipeline::GLUniform::setUniform1i(GLint x) const
{
    glUniform1i(_location, x);
}

void GLPipeline::GLUniform::setUniform1f(GLfloat x) const
{
    glUniform1f(_location, x);
}

void GLPipeline::GLUniform::setUniform2f(GLfloat x, GLfloat y) const
{
    glUniform2f(_location, x, y);
}

void GLPipeline::GLUniform::setUniform3f(GLfloat x, GLfloat y, GLfloat z) const
{
    glUniform3f(_location, x, y, z);
}

void GLPipeline::GLUniform::setUniform4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a) const
{
    glUniform4f(_location, r, g, b, a);
}

void GLPipeline::GLUniform::setUniform4fv(GLsizei count, GLfloat* value) const
{
    glUniform4fv(_location, count, value);
}

void GLPipeline::GLUniform::setUniformColor4f(const Color& color) const
{
    glUniform4f(_location, color.r(), color.g(), color.b(), color.a());
}

void GLPipeline::GLUniform::setUniformMatrix4fv(GLsizei count, GLboolean transpose, const GLfloat* value) const
{
    glUniformMatrix4fv(_location, count, transpose, value);
}

GLPipeline::Shader::Shader(const sp<Recycler>& recycler, uint32_t version, GLenum type, const String& source)
    : _recycler(recycler), _id(compile(version, type, source))
{
}

GLPipeline::Shader::~Shader()
{
    uint32_t id = _id;
    _recycler->recycle([id](GraphicsContext&) {
        LOGD("glDeleteShader(%d)", id);
        glDeleteShader(id);
    });
}

uint32_t GLPipeline::Shader::id()
{
    return _id;
}

GLuint GLPipeline::Shader::compile(uint32_t version, GLenum type, const String& source)
{
    const String versionSrc = source.startsWith("#version ") ? "" : Platform::glShaderVersionDeclaration(version);
    GLuint id = glCreateShader(type);
    const GLchar* src[16] = {versionSrc.c_str()};
    uint32_t slen = Platform::glPreprocessShader(source, &src[1], 15);
    glShaderSource(id, slen + 1, src, nullptr);
    glCompileShader(id);
    GLint compiled;
    glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);
    if(!compiled)
    {
        GLint length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

        size_t len = static_cast<size_t>(length);
        std::vector<GLchar> logs(len + 1);
        glGetShaderInfoLog(id, length, &length, logs.data());
        logs.back() = 0;
        StringBuffer sb;
        for(uint32_t i = 0; i <= slen; i++)
            sb << src[i] << '\n';
        DFATAL("%s\n\n%s", logs.data(), sb.str().c_str());
        return 0;
    }
    return id;
}

GLPipeline::GLRenderCommand::GLRenderCommand(GLenum mode)
    : _mode(mode)
{
}

GLPipeline::GLDrawElements::GLDrawElements(GLenum mode)
    : GLRenderCommand(mode)
{
}

void GLPipeline::GLDrawElements::draw(GraphicsContext& /*graphicsContext*/)
{
    DASSERT(_parameters._count);
    glDrawElements(_mode, static_cast<GLsizei>(_parameters._count), GLIndexType, reinterpret_cast<GLvoid*>(_parameters._start * sizeof(element_index_t)));
}

GLPipeline::GLDrawElementsInstanced::GLDrawElementsInstanced(GLenum mode)
    : GLRenderCommand(mode)
{
}

void GLPipeline::GLDrawElementsInstanced::draw(GraphicsContext& /*graphicsContext*/)
{
    DASSERT(_parameters._count);
    glDrawElementsInstanced(_mode, static_cast<GLsizei>(_parameters._count), GLIndexType, nullptr, _parameters._instance_count);
}

}
}
