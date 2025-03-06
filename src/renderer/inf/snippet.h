#pragma once

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/inf/draw_decorator.h"

namespace ark {

class ARK_API Snippet {
public:
    virtual ~Snippet() = default;

    virtual void preInitialize(PipelineBuildingContext& context) {}
    virtual void preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context, const PipelineLayout& pipelineLayout) {}

    virtual sp<DrawDecorator> makeDrawDecorator(const RenderRequest& /*renderRequest*/) { return sp<DrawDecorator>::make(); }
};

}
