#pragma once

#include "core/base/api.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Snippet {
public:
    virtual ~Snippet() = default;

    virtual void preInitialize(PipelineBuildingContext& context) {}
    virtual void preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context, const PipelineDescriptor& pipelineDescriptor) {}
};

}
