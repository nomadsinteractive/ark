#include "renderer/base/gl_program.h"

#include "core/util/log.h"
#include "core/util/strings.h"

#include "graphics/base/color.h"
#include "renderer/util/gl_debug.h"
#include "platform/platform.h"

namespace ark {

GLProgram::GLProgram(uint32_t version, const String& vertexShader, const String& fragmentShader)
    : _version(version), _vertex_shader(vertexShader), _fragment_shader(fragmentShader), _id(0), _vertex_shader_id(0), _fragment_shader_id(0)
{
}

GLProgram::~GLProgram()
{
    dispose();
}

uint32_t GLProgram::id()
{
    return _id;
}

void GLProgram::prepare(GraphicsContext& /*graphicsContext*/)
{
    _vertex_shader_id = compile(GL_VERTEX_SHADER, _vertex_shader);
    _fragment_shader_id = compile(GL_FRAGMENT_SHADER, _fragment_shader);
    _id = glCreateProgram();
    glAttachShader(_id, _vertex_shader_id);
    glAttachShader(_id, _fragment_shader_id);
    glLinkProgram(_id);
    LOGD("GLProgram[%d]: vertex-shader: %d, fragment-shader: %d", _id, _vertex_shader_id, _fragment_shader_id);
}

void GLProgram::recycle(GraphicsContext& /*graphicsContext*/)
{
    dispose();
}

void GLProgram::glColor(const Color& color)
{
    const GLProgram::Uniform& uColor = getUniform("u_Color");
    uColor.setUniform4f(color.red(), color.green(), color.blue(), color.alpha());
}

GLint GLProgram::getAttribLocation(const String& name)
{
    return glGetAttribLocation(_id, name.c_str());
}

GLint GLProgram::getUniformLocation(const String& name)
{
    GLint location = glGetUniformLocation(_id, name.c_str());
    DWARN(location != -1, "Undefine uniform \"%s\". It might be optimized out, or something goes wrong.", name.c_str());
    return location;
}

const GLProgram::Attribute& GLProgram::getAttribute(const String& name)
{
    const auto iter = _attributes.find(name);
    if(iter != _attributes.end())
        return iter->second;
    _attributes[name] = getAttribLocation(name);
    return _attributes[name];
}

const GLProgram::Uniform& GLProgram::getUniform(const String& name)
{
    const auto iter = _uniforms.find(name);
    if(iter != _uniforms.end())
        return iter->second;
    _uniforms[name] = getUniformLocation(name);
    return _uniforms[name];
}

GLuint GLProgram::compile(GLenum type, const String& shader_source)
{
    const String versionSrc = shader_source.startsWith("#version ") ? "" : Platform::glShaderVersionDeclaration(_version);
    GLuint shader = glCreateShader(type);
    const GLchar* src[16] = {versionSrc.c_str()};
    uint32_t slen = Platform::glPreprocessShader(shader_source, &src[1], 15);
    glShaderSource(shader, slen + 1, src, nullptr);
    glCompileShader(shader);
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if(!compiled)
    {
        GLint length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        String log(length, ' ');
        glGetShaderInfoLog(shader, length, &length, (GLchar*) log.c_str());
        StringBuffer sb;
        for(uint32_t i = 0; i <= slen; i++)
            sb << src[i] << '\n';
        DFATAL("%s\n\n%s", log.c_str(), sb.str().c_str());
        return 0;
    }
    return shader;
}

void GLProgram::dispose()
{
    LOGD("GLProgram[%d]: vertex-shader: %d, fragment-shader: %d", _id, _vertex_shader_id, _fragment_shader_id);
    if(_vertex_shader_id)
        glDeleteShader(_vertex_shader_id);
    if(_fragment_shader_id)
        glDeleteShader(_fragment_shader_id);
    if(_id)
        glDeleteProgram(_id);
    _vertex_shader_id = _fragment_shader_id = _id = 0;
    _attributes.clear();
    _uniforms.clear();
}

void GLProgram::use() const
{
    glUseProgram(_id);
}

void GLProgram::Attribute::setVertexPointer(GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer) const
{
    glEnableVertexAttribArray(_location);
    glVertexAttribPointer(_location, size, type, normalized, stride, pointer);
}

GLProgram::Attribute::Attribute(GLuint location)
    : _location(location)
{
}

GLProgram::Attribute::Attribute(const GLProgram::Attribute& other)
    : _location(other._location)
{
}

GLuint GLProgram::Attribute::location() const
{
    return _location;
}

GLProgram::Uniform::Uniform(GLint location)
    : _location(location), _last_modified(0)
{
}

GLProgram::Uniform::Uniform(const GLProgram::Uniform& other)
    : _location(other._location), _last_modified(other._last_modified)
{
}

GLProgram::Uniform::operator bool() const
{
    return _location != -1;
}

void GLProgram::Uniform::setUniform1i(GLint x) const
{
    glUniform1i(_location, x);
}

void GLProgram::Uniform::setUniform1f(GLfloat x) const
{
    glUniform1f(_location, x);
}

void GLProgram::Uniform::setUniform2f(GLfloat x, GLfloat y) const
{
    glUniform2f(_location, x, y);
}

void GLProgram::Uniform::setUniform3f(GLfloat x, GLfloat y, GLfloat z) const
{
    glUniform3f(_location, x, y, z);
}

void GLProgram::Uniform::setUniform4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a) const
{
    glUniform4f(_location, r, g, b, a);
}

void GLProgram::Uniform::setUniform4fv(GLsizei count, GLfloat* value) const
{
    glUniform4fv(_location, count, value);
}

void GLProgram::Uniform::setUniformColor4f(const Color& color) const
{
    glUniform4f(_location, color.red(), color.green(), color.blue(), color.alpha());
}

void GLProgram::Uniform::setUniformMatrix4fv(GLsizei count, GLboolean transpose, const GLfloat* value) const
{
    glUniformMatrix4fv(_location, count, transpose, value);
}

void GLProgram::Uniform::setUniformMatrix4fv(GLsizei count, GLboolean transpose, const GLfloat* value, uint64_t timestamp) const
{
    if(timestamp > _last_modified)
    {
        glUniformMatrix4fv(_location, count, transpose, value);
        _last_modified = timestamp;
    }
}
}
