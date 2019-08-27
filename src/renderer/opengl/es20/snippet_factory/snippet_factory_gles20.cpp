#include "renderer/opengl/es20/snippet_factory/snippet_factory_gles20.h"

#include "renderer/base/drawing_context.h"
#include "renderer/base/shader.h"
#include "renderer/inf/pipeline.h"
#include "renderer/inf/snippet.h"

#include "renderer/base/shader_bindings.h"
#include "renderer/opengl/base/gl_pipeline.h"

#include "platform/gl/gl.h"

namespace ark {
namespace gles20 {

namespace {

class SnippetGLES20 : public Snippet {
public:
    virtual void preDraw(GraphicsContext& graphicsContext, const DrawingContext& context) override {
        glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(context._vertex_buffer.id()));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(context._index_buffer.id()));
        const sp<opengl::GLPipeline> pipeline = context._shader_bindings->getPipeline(graphicsContext);
        pipeline->bindBuffer(graphicsContext, context._shader_bindings->pipelineInput(), context._shader_bindings->divisors());
    }

};

}

sp<Snippet> SnippetFactoryGLES20::createCoreSnippet(RenderController& /*renderController*/)
{
    return sp<SnippetGLES20>::make();
}

}
}
