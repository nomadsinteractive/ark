#ifndef ARK_RENDERER_INF_PIPELINE_H_
#define ARK_RENDERER_INF_PIPELINE_H_

#include "core/base/api.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/texture.h"
#include "renderer/inf/resource.h"

namespace ark {

class ARK_API PipelineOperation {
public:
    virtual ~PipelineOperation() = default;

    virtual void bind(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) = 0;
    virtual void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) = 0;

    virtual void compute(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) = 0;
};

class ARK_API Pipeline : public Resource, public PipelineOperation {
public:
    virtual ~Pipeline() = default;
};

}

#endif
