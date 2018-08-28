#ifndef ARK_RENDERER_BASE_GL_PROGRAM_H_
#define ARK_RENDERER_BASE_GL_PROGRAM_H_

#include <map>

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/inf/gl_resource.h"

#include "platform/gl/gl.h"

namespace ark {

class ARK_API GLProgram : public GLResource {
public:
    GLProgram(const sp<GLRecycler>& recycler, uint32_t version, const String& vertexShader, const String& fragmentShader);
    ~GLProgram();

    virtual uint32_t id() override;
    virtual void prepare(GraphicsContext& graphicsContext) override;
    virtual void recycle(GraphicsContext& graphicsContext) override;

    class ARK_API Attribute {
    public:
        Attribute(GLint location = -1);
        Attribute(const Attribute& other) = default;

        GLint location() const;

        void setVertexPointer(GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer) const;

    private:
        GLint _location;
    };

    class ARK_API Uniform {
    public:
        Uniform(GLint location = -1);
        Uniform(const Uniform& other);

        explicit operator bool() const;

        void setUniform1i(GLint x) const;
        void setUniform1f(GLfloat x) const;
        void setUniform2f(GLfloat x, GLfloat y) const;
        void setUniform3f(GLfloat x, GLfloat y, GLfloat z) const;
        void setUniform4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a) const;
        void setUniform4fv(GLsizei count, GLfloat* value) const;
        void setUniformColor4f(const Color& color) const;
        void setUniformMatrix4fv(GLsizei count, GLboolean transpose, const GLfloat* value) const;
        void setUniformMatrix4fv(GLsizei count, GLboolean transpose, const GLfloat* value, uint64_t timestamp) const;

    private:
        GLint _location;
        mutable uint64_t _last_modified;
    };

    class ARK_API Shader {
    public:
        Shader(const sp<GLRecycler>& recycler, uint32_t version, GLenum type, const String& source);
        ~Shader();

        uint32_t id();

    private:
        GLuint compile(uint32_t version, GLenum type, const String& source);

    private:
        sp<GLRecycler> _recycler;
        uint32_t _id;
    };

    GLint getAttribLocation(const String& name);
    GLint getUniformLocation(const String& name);

    const Attribute& getAttribute(const String& name);
    const Uniform& getUniform(const String& name);

    void use() const;
    void validate(GraphicsContext& graphicsContext) const;

private:
    String getInformationLog() const;

private:
    sp<GLRecycler> _recycler;

    uint32_t _version;

    String _vertex_source;
    String _fragment_source;

    GLuint _id;

    sp<GLProgram::Shader> _vertex_shader;
    sp<GLProgram::Shader> _fragment_shader;

    std::map<String, Attribute> _attributes;
    std::map<String, Uniform> _uniforms;

};

}

#endif
