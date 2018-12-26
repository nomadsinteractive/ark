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

    virtual sp<Pipeline> buildPipeline(GraphicsContext& graphicsContext, const sp<ShaderBindings>& shaderBindings) = 0;

};

}

#endif
