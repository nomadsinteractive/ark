#include "opengl/impl/pipeline_factory/pipeline_factory_opengl.h"

#include "core/types/shared_ptr.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/render_engine_context.h"

#include "opengl/base/gl_pipeline.h"

namespace ark::plugin::opengl {

sp<Pipeline> PipelineFactoryOpenGL::buildPipeline(GraphicsContext& graphicsContext, const PipelineBindings& pipelineBindings, Map<enums::ShaderStageBit, String> stages)
{
    const sp<RenderEngineContext>& renderContext = graphicsContext.renderContext();
    return sp<Pipeline>::make<GLPipeline>(graphicsContext.recycler(), renderContext->getGLSLVersion(), std::move(stages), pipelineBindings);
}

}
