#include "renderer/opengl/es20/snippet_factory/snippet_factory_gles20.h"

#include "renderer/base/drawing_context.h"
#include "renderer/base/shader.h"
#include "renderer/inf/pipeline.h"
#include "renderer/inf/snippet.h"

#include "renderer/opengl/base/gl_pipeline.h"

#include "platform/gl/gl.h"

namespace ark {
namespace gles20 {

namespace {

class SnippetGLES20 : public Snippet {
public:

    virtual void preDraw(GraphicsContext& graphicsContext, Shader& shader, const DrawingContext& context) override {
        glBindBuffer(GL_ARRAY_BUFFER, context._array_buffer.id());
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, context._index_buffer.id());
        const sp<opengl::GLPipeline> pipeline = shader.pipeline();
        pipeline->bindBuffer(graphicsContext, context._shader_bindings);
    }

};

}

sp<Snippet> SnippetFactoryGLES20::createCoreSnippet(RenderController& /*glResourceManager*/, const sp<PipelineFactory>& /*pipelineFactory*/, const sp<ShaderBindings>& /*shaderBindings*/)
{
    return sp<SnippetGLES20>::make();
}

}
}
