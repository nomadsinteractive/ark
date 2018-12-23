#include "renderer/opengl/pipeline_factory/pipeline_factory_opengl.h"

#include "core/types/shared_ptr.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/resource_manager.h"
#include "renderer/base/gl_context.h"
#include "renderer/base/pipeline_layout.h"

#include "renderer/opengl/base/gl_pipeline.h"
#include "renderer/opengl/render_command/gl_draw_elements.h"
#include "renderer/opengl/render_command/gl_draw_elements_instanced.h"

namespace ark {
namespace opengl {

sp<Pipeline> PipelineFactoryOpenGL::buildPipeline(GraphicsContext& graphicsContext, const PipelineLayout& pipelineLayout, const ShaderBindings& /*bindings*/)
{
    const sp<GLContext>& glContext = graphicsContext.glContext();
    return sp<GLPipeline>::make(graphicsContext.resourceManager()->recycler(), glContext->getGLSLVersion(),
                                pipelineLayout.vertex().process(glContext), pipelineLayout.fragment().process(glContext));
}

sp<RenderCommand> PipelineFactoryOpenGL::buildRenderCommand(ObjectPool& objectPool, DrawingContext drawingContext, const sp<Shader>& shader, RenderModel::Mode mode, int32_t count)
{
    static const GLenum models[RenderModel::RENDER_MODE_COUNT] = {GL_LINES, GL_POINTS, GL_TRIANGLES, GL_TRIANGLE_STRIP};
    if(count > 0)
        return objectPool.obtain<GLDrawElementsInstanced>(std::move(drawingContext), shader, models[mode], count);
    return objectPool.obtain<GLDrawElements>(std::move(drawingContext), shader, models[mode]);
}

}
}
