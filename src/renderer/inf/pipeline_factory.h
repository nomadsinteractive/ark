#ifndef ARK_RENDERER_INF_PIPELINE_FACTORY_H_
#define ARK_RENDERER_INF_PIPELINE_FACTORY_H_

#include "core/base/api.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/inf/render_model.h"

namespace ark {

class ARK_API PipelineFactory {
public:
    virtual ~PipelineFactory() = default;

    virtual sp<Pipeline> buildPipeline(GraphicsContext& graphicsContext, const PipelineLayout& pipelineLayout) = 0;
    virtual sp<RenderCommand> buildRenderCommand(ObjectPool& objectPool, DrawingContext drawingContext, const sp<Shader>& shader, RenderModel::Mode mode, int32_t count) = 0;

};

}

#endif
