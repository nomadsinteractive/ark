#pragma once

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/shader_preprocessor.h"
#include "renderer/inf/pipeline.h"
#include "renderer/inf/snippet.h"

#include "platform/gl/gl.h"

namespace ark::plugin::opengl {

class GLPipeline final : public Pipeline {
public:
    GLPipeline(const sp<Recycler>& recycler, uint32_t version, Map<enums::ShaderStageBit, String> stages, const PipelineBindings& bindings);
    ~GLPipeline() override;

    uint64_t id() override;
    void upload(GraphicsContext& graphicsContext) override;
    ResourceRecycleFunc recycle() override;

    void bind(GraphicsContext& graphicsContext, const DrawingContext& drawingContext);
    void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override;
    void compute(GraphicsContext& graphicsContext, const ComputeContext& computeContext) override;

    void bindBuffer(GraphicsContext& graphicsContext, const PipelineLayout& shaderLayout, const Vector<std::pair<uint32_t, Buffer>>& streams);

    void activeTexture(const Texture& texture, const String& name, uint32_t binding) const;

    class GLUniform {
    public:
        GLUniform(GLint location = -1);
        GLUniform(const GLUniform& other) = default;

        explicit operator bool() const;

        void setUniform1i(GLint x) const;
        void setUniform2i(GLint x, GLint y) const;
        void setUniform1f(GLfloat x) const;
        void setUniform2f(GLfloat x, GLfloat y) const;
        void setUniform3f(GLfloat x, GLfloat y, GLfloat z) const;
        void setUniform4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a) const;
        void setUniform4fv(GLsizei count, const GLfloat* value) const;
        void setUniformColor4f(const Color& color) const;
        void setUniformMatrix4fv(GLsizei count, GLboolean transpose, const GLfloat* value) const;

    private:
        GLint _location;
    };

    const GLUniform& getUniform(const String& name) const;

    struct Stub;

private:
    void bindBuffer(GraphicsContext&, const PipelineLayout& shaderLayout, uint32_t divisor) const;
    sp<PipelineOperation> makePipelineOperation(const PipelineBindings& pipelineBindings) const;

private:
    sp<Stub> _stub;
    sp<Recycler> _recycler;

    uint32_t _version;

    Map<enums::ShaderStageBit, String> _stages;

    sp<PipelineOperation> _pipeline_operation;
    Vector<sp<DrawDecorator>> _draw_decorators;
};

}
