#include "renderer/opengl/pipeline_factory/pipeline_factory_opengl.h"

#include "core/types/shared_ptr.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/render_context.h"
#include "renderer/base/render_controller.h"

#include "renderer/opengl/base/gl_pipeline.h"

namespace ark {
namespace opengl {

sp<Pipeline> PipelineFactoryOpenGL::buildPipeline(GraphicsContext& graphicsContext, const PipelineBindings& bindings)
{
    DCHECK(bindings.mode() != ModelLoader::RENDER_MODE_NONE, "Pipeline has no RenderMode initialized");
    const sp<PipelineLayout>& pipelineLayout = bindings.layout();
    const sp<RenderContext>& renderContext = graphicsContext.renderContext();
    return sp<GLPipeline>::make(graphicsContext.recycler(), renderContext->getGLSLVersion(),
                                pipelineLayout->vertex().process(renderContext), pipelineLayout->fragment().process(renderContext), bindings);
}

}
}
