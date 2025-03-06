#pragma once

#include "core/base/api.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API DrawDecorator {
public:
    virtual ~DrawDecorator() = default;

    virtual void preDraw(GraphicsContext& graphicsContext, const DrawingContext& context) {}
    virtual void postDraw(GraphicsContext& graphicsContext, const DrawingContext& context) {}
};

}