#ifndef ARK_RENDERER_OPENGL_BASE_GL_PIPELINE_H_
#define ARK_RENDERER_OPENGL_BASE_GL_PIPELINE_H_

#include <map>

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/render_command.h"

#include "renderer/forwarding.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/inf/pipeline.h"
#include "renderer/inf/resource.h"
#include "renderer/inf/snippet.h"

#include "platform/gl/gl.h"

namespace ark {
namespace opengl {

class GLPipeline : public Pipeline {
public:
    GLPipeline(const sp<Recycler>& recycler, uint32_t version, std::map<PipelineInput::ShaderStage, String> shaders, const PipelineBindings& bindings);
    virtual ~GLPipeline() override;

    virtual uint64_t id() override;
    virtual void upload(GraphicsContext& graphicsContext, const sp<Uploader>& uploader) override;
    virtual RecycleFunc recycle() override;

    virtual void bind(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override;
    virtual void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override;
    virtual void compute(GraphicsContext& graphicsContext, const ComputeContext& computeContext) override;

    void bindBuffer(GraphicsContext& graphicsContext, const PipelineInput& input, const std::map<uint32_t, Buffer>& divisors);

    void activeTexture(const Texture& texture, uint32_t name);

    class GLUniform {
    public:
        GLUniform(GLint location = -1);
        GLUniform(const GLUniform& other) = default;

        explicit operator bool() const;

        void setUniform1i(GLint x) const;
        void setUniform1f(GLfloat x) const;
        void setUniform2f(GLfloat x, GLfloat y) const;
        void setUniform3f(GLfloat x, GLfloat y, GLfloat z) const;
        void setUniform4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a) const;
        void setUniform4fv(GLsizei count, GLfloat* value) const;
        void setUniformColor4f(const Color& color) const;
        void setUniformMatrix4fv(GLsizei count, GLboolean transpose, const GLfloat* value) const;

    private:
        GLint _location;
    };

    const GLUniform& getUniform(const String& name);

private:
    class GLAttribute {
    public:
        GLAttribute(GLint location = -1);
        GLAttribute(const GLAttribute& other) = default;

        void bind(const Attribute& attribute, GLsizei stride) const;

    private:
        void setVertexPointer(const Attribute& attribute, GLuint location, GLsizei stride, uint32_t length, uint32_t offset) const;

    private:
        GLint _location;
    };

    class Stage {
    public:
        Stage(const sp<Recycler>& recycler, uint32_t version, GLenum type, const String& source);
        ~Stage();

        uint32_t id();

    private:
        GLuint compile(uint32_t version, GLenum type, const String& source);

    private:
        sp<Recycler> _recycler;
        uint32_t _id;
    };

private:
    String getInformationLog(GLuint id) const;
    sp<Stage> makeShader(GraphicsContext& graphicsContext, uint32_t version, GLenum type, const String& source) const;

    void bindBuffer(GraphicsContext&, const PipelineInput& input, uint32_t divisor);

    class BakedRenderer {
    public:
        virtual ~BakedRenderer() = default;

        virtual void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) = 0;
    };

    class GLDrawArrays : public BakedRenderer {
    public:
        GLDrawArrays(GLenum mode);

        virtual void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override;

    private:
        GLenum _mode;
    };

    class GLDrawElements : public BakedRenderer {
    public:
        GLDrawElements(GLenum mode);

        virtual void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override;

    private:
        GLenum _mode;
    };

    class GLDrawElementsInstanced : public BakedRenderer {
    public:
        GLDrawElementsInstanced(GLenum mode);

        virtual void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override;

    private:
        GLenum _mode;
    };

    class GLMultiDrawElementsIndirect : public BakedRenderer {
    public:
        GLMultiDrawElementsIndirect(GLenum mode);

        virtual void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override;

    private:
        GLenum _mode;
    };

    struct Stub {
        Stub();

        void bindUBO(const RenderLayer::UBOSnapshot& uboSnapshot, const sp<PipelineInput::UBO>& ubo);
        void bindUniform(float* buf, uint32_t size, const Uniform& uniform);
        void activeTexture(const Texture& texture, uint32_t name);

        const GLPipeline::GLUniform& getUniform(const String& name);
        GLint getUniformLocation(const String& name);

        const GLPipeline::GLAttribute& getAttribute(const String& name);
        GLint getAttribLocation(const String& name);

        void bindUBOSnapshots(const std::vector<RenderLayer::UBOSnapshot>& uboSnapshots, const PipelineInput& pipelineInput);

        GLuint _id;

        std::map<String, GLAttribute> _attributes;
        std::map<String, GLUniform> _uniforms;

        bool _rebind_needed;
    };

    class PipelineOperationDraw : public PipelineOperation {
    public:
        PipelineOperationDraw(const sp<Stub>& stub, const PipelineBindings& bindings);

        virtual void bind(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override;
        virtual void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override;
        virtual void compute(GraphicsContext& graphicsContext, const ComputeContext& computeContext) override;

    private:
        sp<GLPipeline::BakedRenderer> makeBakedRenderer(const PipelineBindings& bindings) const;

    private:
        sp<Stub> _stub;

        bool _cull_face;

        Rect _scissor;
        bool _scissor_enabled;

        sp<BakedRenderer> _renderer;
    };

    class PipelineOperationCompute : public PipelineOperation {
    public:
        PipelineOperationCompute(const sp<Stub>& stub);

        virtual void bind(GraphicsContext& graphicsContext, const DrawingContext& computeContext) override;
        virtual void draw(GraphicsContext& graphicsContext, const DrawingContext& computeContext) override;
        virtual void compute(GraphicsContext& graphicsContext, const ComputeContext& computeContext) override;

    private:
        sp<Stub> _stub;

    };

    sp<PipelineOperation> makePipelineOperation(const PipelineBindings& bindings) const;

private:
    sp<Stub> _stub;
    sp<Recycler> _recycler;

    uint32_t _version;

    std::map<PipelineInput::ShaderStage, String> _shaders;

    sp<PipelineOperation> _pipeline_operation;
    std::vector<sp<Snippet::DrawEvents>> _draw_tests;
};

}
}

#endif
