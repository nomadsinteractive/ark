#include "renderer/opengl/pipeline_factory/pipeline_factory_opengl.h"

#include "core/types/shared_ptr.h"

#include "renderer/base/gl_context.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/resource_manager.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/shader_bindings.h"

#include "renderer/opengl/base/gl_pipeline.h"

namespace ark {
namespace opengl {

sp<Pipeline> PipelineFactoryOpenGL::buildPipeline(GraphicsContext& graphicsContext, const sp<ShaderBindings>& shaderBindings)
{
    DCHECK(shaderBindings->renderMode() != RenderModel::RENDER_MODE_NONE, "Shader has no RenderModel initialized");
    const sp<PipelineLayout>& pipelineLayout = shaderBindings->pipelineLayout();
    const sp<GLContext>& glContext = graphicsContext.glContext();
    return sp<GLPipeline>::make(graphicsContext.resourceManager()->recycler(), glContext->getGLSLVersion(),
                                pipelineLayout->vertex().process(glContext), pipelineLayout->fragment().process(glContext), shaderBindings);
}

}
}
