#pragma once

#include "core/base/api.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Snippet {
public:
    virtual ~Snippet() = default;

    class ARK_API DrawEvents {
    public:
        virtual ~DrawEvents() = default;

        virtual void preDraw(GraphicsContext& graphicsContext, const DrawingContext& context) {}
        virtual void postDraw(GraphicsContext& graphicsContext) {}
    };

    virtual void preInitialize(PipelineBuildingContext& context) {}
    virtual void preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context, const PipelineLayout& pipelineLayout) {}

    virtual sp<DrawEvents> makeDrawEvents(const RenderRequest& /*renderRequest*/) { return makeDrawEvents(); }
    virtual sp<DrawEvents> makeDrawEvents() { return nullptr; }
};

}
