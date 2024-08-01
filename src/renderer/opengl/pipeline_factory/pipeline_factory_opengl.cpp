#include "renderer/opengl/pipeline_factory/pipeline_factory_opengl.h"

#include "core/types/shared_ptr.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/render_engine_context.h"

#include "renderer/opengl/base/gl_pipeline.h"

namespace ark::opengl {

sp<Pipeline> PipelineFactoryOpenGL::buildPipeline(GraphicsContext& graphicsContext, const PipelineBindings& bindings)
{
    const sp<PipelineLayout>& pipelineLayout = bindings.pipelineLayout();
    const sp<RenderEngineContext>& renderContext = graphicsContext.renderContext();
    return sp<Pipeline>::make<GLPipeline>(graphicsContext.recycler(), renderContext->getGLSLVersion(), pipelineLayout->getPreprocessedShaders(graphicsContext.renderContext()), bindings.pipelineDescriptor());
}

}
