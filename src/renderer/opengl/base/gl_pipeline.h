#ifndef ARK_RENDERER_OPENGL_BASE_GL_PIPELINE_H_
#define ARK_RENDERER_OPENGL_BASE_GL_PIPELINE_H_

#include <map>

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/inf/render_resource.h"
#include "renderer/inf/pipeline.h"

#include "platform/gl/gl.h"

namespace ark {

class ARK_API GLPipeline : public Pipeline {
public:
    GLPipeline(const sp<GLRecycler>& recycler, uint32_t version, const String& vertexShader, const String& fragmentShader);
    ~GLPipeline() override;

    virtual uint32_t id() override;
    virtual void upload(GraphicsContext& graphicsContext) override;
    virtual Recycler recycle() override;

    virtual void use() override;
    virtual void bind(GraphicsContext& graphicsContext, const ShaderBindings& bindings) override;
    virtual void bindUniform(GraphicsContext& graphicsContext, const Uniform& uniform) override;
    virtual void activeTexture(RenderResource& texture, uint32_t target, uint32_t name) override;

    void glUpdateMatrix(GraphicsContext& graphicsContext, const String& name, const Matrix& matrix);

    class ARK_API GLUniform {
    public:
        GLUniform(GLint location = -1);
        GLUniform(const GLUniform& other);

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

    const GLUniform& getUniform(const String& name);

private:
    class Attribute {
    public:
        Attribute(GLint location = -1);
        Attribute(const Attribute& other) = default;

        GLint location() const;

        void setVertexPointer(GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer) const;

    private:
        GLint _location;
    };

    class Shader {
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

private:
    String getInformationLog() const;
    sp<Shader> makeShader(GraphicsContext& graphicsContext, uint32_t version, GLenum type, const String& source) const;

    GLint getAttribLocation(const String& name);
    const Attribute& getAttribute(const String& name);

    GLint getUniformLocation(const String& name);

    void bind(GraphicsContext&, const PipelineInput& input, uint32_t divisor);

private:
    sp<GLRecycler> _recycler;

    uint32_t _version;

    String _vertex_source;
    String _fragment_source;

    GLuint _id;

    sp<GLPipeline::Shader> _vertex_shader;
    sp<GLPipeline::Shader> _fragment_shader;

    std::map<String, Attribute> _attributes;
    std::map<String, GLUniform> _uniforms;

};

}

#endif