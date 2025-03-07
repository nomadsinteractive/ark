#pragma once

#include "core/types/shared_ptr.h"

#include "renderer/inf/draw_decorator.h"

namespace ark {

class DrawDecoratorComposite final : public DrawDecorator {
public:
    DrawDecoratorComposite(sp<DrawDecorator> delegate, sp<DrawDecorator> next);

    void preDraw(GraphicsContext& graphicsContext, const DrawingContext& context) override;
    void postDraw(GraphicsContext& graphicsContext, const DrawingContext& context) override;

    static sp<DrawDecorator> compose(sp<DrawDecorator> self, sp<DrawDecorator> next);

private:
    sp<DrawDecorator> _delegate;
    sp<DrawDecorator> _next;
};

}
