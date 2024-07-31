#include "renderer/opengl/pipeline_factory/pipeline_factory_opengl.h"

#include "core/types/shared_ptr.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/base/render_controller.h"

#include "renderer/opengl/base/gl_pipeline.h"

namespace ark::opengl {

sp<Pipeline> PipelineFactoryOpenGL::buildPipeline(GraphicsContext& graphicsContext, const PipelineDescriptor& bindings)
{
    const sp<PipelineLayout>& pipelineLayout = bindings.layout();
    const sp<RenderEngineContext>& renderContext = graphicsContext.renderContext();
    return sp<GLPipeline>::make(graphicsContext.recycler(), renderContext->getGLSLVersion(), pipelineLayout->getPreprocessedShaders(graphicsContext.renderContext()), bindings);
}

}
