#include "renderer/opengl/pipeline_factory/pipeline_factory_opengl.h"

#include "core/types/shared_ptr.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/render_engine_context.h"

#include "renderer/opengl/base/gl_pipeline.h"

namespace ark::opengl {

sp<Pipeline> PipelineFactoryOpenGL::buildPipeline(GraphicsContext& graphicsContext, const PipelineDescriptor& pipelineDescriptor, std::map<Enum::ShaderStageBit, String> stages)
{
    const sp<PipelineLayout>& pipelineLayout = pipelineDescriptor.layout();
    const sp<RenderEngineContext>& renderContext = graphicsContext.renderContext();
    return sp<Pipeline>::make<GLPipeline>(graphicsContext.recycler(), renderContext->getGLSLVersion(), pipelineLayout->getPreprocessedStages(graphicsContext.renderContext()), pipelineDescriptor);
}

}
