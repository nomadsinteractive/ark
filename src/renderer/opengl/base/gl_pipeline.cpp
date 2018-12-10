#include "renderer/opengl/base/gl_pipeline.h"

#include "core/inf/flatable.h"
#include "core/types/weak_ptr.h"
#include "core/util/log.h"
#include "core/util/strings.h"

#include "graphics/base/color.h"
#include "graphics/base/matrix.h"

#include "renderer/base/recycler.h"
#include "renderer/base/resource_manager.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/texture.h"
#include "renderer/base/uniform.h"

#include "renderer/opengl/util/gl_debug.h"

#include "platform/platform.h"

namespace ark {

GLPipeline::GLPipeline(const sp<Recycler>& recycler, uint32_t version, const String& vertexShader, const String& fragmentShader)
    : _recycler(recycler), _version(version), _vertex_source(vertexShader), _fragment_source(fragmentShader), _id(0)
{
}

GLPipeline::~GLPipeline()
{
    if(_id)
    {
        uint32_t id = _id;
        _recycler->recycle([id](GraphicsContext&) {
            LOGD("glDeleteProgram(%d)", id);
            glDeleteProgram(id);
        });
    }
}

uint32_t GLPipeline::id()
{
    return _id;
}

void GLPipeline::upload(GraphicsContext& graphicsContext)
{
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

void GLPipeline::bind(GraphicsContext& graphicsContext, const ShaderBindings& bindings)
{
    DCHECK(id(), "GLProgram unprepared");
    bind(graphicsContext, bindings.pipelineInput(), 0);
    for(const auto& i : bindings.instancedArrays())
    {
        glBindBuffer(GL_ARRAY_BUFFER, i.second.id());
        bind(graphicsContext, bindings.pipelineInput(), i.first);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void GLPipeline::bindUniform(GraphicsContext& /*graphicsContext*/, const Uniform& uniform)
{
    const GLPipeline::GLUniform& glUniform = getUniform(uniform.name());
    float buf[1024];
    const sp<Flatable>& flatable = uniform.flatable();
    uint32_t size = flatable->size();
    DCHECK(size <= sizeof(buf), "Size too large: %d", size);
    flatable->flat(buf);
    switch(uniform.type()) {
    case Uniform::UNIFORM_I1:
        DCHECK(flatable->size() == 4, "Wrong uniform1i size: %d", size);
        glUniform.setUniform1i(*reinterpret_cast<int32_t*>(buf));
        break;
    case Uniform::UNIFORM_F1:
        DCHECK(flatable->size() == 4, "Wrong uniform1f size: %d", size);
        glUniform.setUniform1f(buf[0]);
        break;
    case Uniform::UNIFORM_F2:
        DCHECK(flatable->size() == 8, "Wrong uniform2f size: %d", size);
        glUniform.setUniform2f(buf[0], buf[1]);
        break;
    case Uniform::UNIFORM_F3:
        DCHECK(flatable->size() == 12, "Wrong uniform3f size: %d", size);
        glUniform.setUniform3f(buf[0], buf[1], buf[2]);
        break;
    case Uniform::UNIFORM_F4:
        DCHECK(flatable->size() == 16, "Wrong uniform4f size: %d", size);
        glUniform.setUniform4f(buf[0], buf[1], buf[2], buf[3]);
        break;
    case Uniform::UNIFORM_F4V:
        DCHECK(flatable->size() % 16 == 0, "Wrong uniform4fv size: %d", size);
        glUniform.setUniform4fv(flatable->size() / 16, buf);
        break;
    case Uniform::UNIFORM_MAT4V:
        DCHECK(flatable->size() % 64 == 0, "Wrong color4fv size: %d", size);
        glUniform.setUniformMatrix4fv(flatable->size() / 64, GL_FALSE, buf);
        break;
    default:
        DFATAL("Unimplemented");
    }
}

void GLPipeline::activeTexture(Resource& texture, uint32_t target, uint32_t name)
{
    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + name));
    glBindTexture(static_cast<GLenum>(target), texture.id());

    char uniformName[16] = "u_Texture0";
    uniformName[9] = static_cast<char>('0' + name);
    const GLPipeline::GLUniform& uTexture = getUniform(uniformName);
    uTexture.setUniform1i(name);
}

void GLPipeline::glUpdateMatrix(GraphicsContext& graphicsContext, const String& name, const Matrix& matrix)
{
    const GLPipeline::GLUniform& uniform = getUniform(name);
    DCHECK(uniform, "Uniform %s not found", name.c_str());
    uniform.setUniformMatrix4fv(1, GL_FALSE, matrix.value(), graphicsContext.tick());
}

void GLPipeline::bind(GraphicsContext& /*graphicsContext*/, const PipelineInput& input, uint32_t divisor)
{
    const PipelineInput::Stream& stream = input.getStream(divisor);
    for(auto iter : stream.attributes())
    {
        const GLPipeline::GLAttribute& glAttribute = getAttribute(iter.second.name());
        glAttribute.bind(iter.second, stream.stride());
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

    String log(length, 0);
    glGetProgramInfoLog(_id, log.length(), &length, (GLchar*) log.c_str());
    return log;
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

    const sp<Shader> shader = sp<Shader>::make(graphicsContext.resourceManager()->recycler(), version, type, source);
    shaders[type][source] = shader;
    return shader;
}

void GLPipeline::use()
{
    glUseProgram(_id);
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

void GLPipeline::GLAttribute::setVertexPointer(const Attribute& attribute, GLint location, GLsizei stride, uint32_t length, uint32_t offset) const
{
    static const GLenum glTypes[Attribute::TYPE_COUNT] = {GL_BYTE, GL_FLOAT, GL_UNSIGNED_INT, GL_SHORT, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT};

    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location, length, glTypes[attribute.type()], attribute.normalized() ? GL_TRUE : GL_FALSE, stride, reinterpret_cast<void*>(offset));
    if(attribute.divisor())
        glVertexAttribDivisor(location, attribute.divisor());
}

GLPipeline::GLUniform::GLUniform(GLint location)
    : _location(location), _last_modified(0)
{
}

GLPipeline::GLUniform::GLUniform(const GLPipeline::GLUniform& other)
    : _location(other._location), _last_modified(other._last_modified)
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
    glUniform4f(_location, color.red(), color.green(), color.blue(), color.alpha());
}

void GLPipeline::GLUniform::setUniformMatrix4fv(GLsizei count, GLboolean transpose, const GLfloat* value) const
{
    glUniformMatrix4fv(_location, count, transpose, value);
}

void GLPipeline::GLUniform::setUniformMatrix4fv(GLsizei count, GLboolean transpose, const GLfloat* value, uint64_t timestamp) const
{
    if(timestamp > _last_modified)
    {
        glUniformMatrix4fv(_location, count, transpose, value);
        _last_modified = timestamp;
    }
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
        String log(length, ' ');
        glGetShaderInfoLog(id, length, &length, (GLchar*) log.c_str());
        StringBuffer sb;
        for(uint32_t i = 0; i <= slen; i++)
            sb << src[i] << '\n';
        DFATAL("%s\n\n%s", log.c_str(), sb.str().c_str());
        return 0;
    }
    return id;
}

}
