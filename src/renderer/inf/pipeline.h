#ifndef ARK_RENDERER_INF_PIPELINE_H_
#define ARK_RENDERER_INF_PIPELINE_H_

#include "core/base/api.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/texture.h"
#include "renderer/inf/resource.h"

namespace ark {

class ARK_API Pipeline : public Resource {
public:
    virtual ~Pipeline() = default;

    virtual void bind(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) = 0;
    virtual void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) = 0;
};

}

#endif
