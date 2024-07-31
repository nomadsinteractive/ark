#pragma once

#include "core/base/api.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API PipelineFactory {
public:
    virtual ~PipelineFactory() = default;

    virtual sp<Pipeline> buildPipeline(GraphicsContext& graphicsContext, const PipelineDescriptor& bindings) = 0;

};

}
