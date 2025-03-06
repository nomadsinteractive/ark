#pragma once

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Snippet {
public:
    virtual ~Snippet() = default;

    class ARK_API DrawDecorator {
    public:
        virtual ~DrawDecorator() = default;

        virtual void preDraw(GraphicsContext& graphicsContext, const DrawingContext& context) {}
        virtual void postDraw(GraphicsContext& graphicsContext, const DrawingContext& context) {}
    };

    virtual void preInitialize(PipelineBuildingContext& context) {}
    virtual void preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context, const PipelineLayout& pipelineLayout) {}

    virtual sp<DrawDecorator> makeDrawDecorator(const RenderRequest& /*renderRequest*/) { return sp<DrawDecorator>::make(); }
};

}
