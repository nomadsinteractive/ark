#ifndef ARK_RENDERER_INF_SNIPPET_H_
#define ARK_RENDERER_INF_SNIPPET_H_

#include "core/base/api.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Snippet {
public:
    virtual ~Snippet() = default;

    virtual void preInitialize(PipelineLayout& source) {}

    virtual void preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context) {}
    virtual void preDraw(GraphicsContext& graphicsContext, const Shader& shader, const DrawingContext& context) {}
    virtual void postDraw(GraphicsContext& graphicsContext) {}
};

}

#endif