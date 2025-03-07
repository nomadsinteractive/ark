#include "renderer/impl/draw_decorator/draw_decorator_composite.h"

namespace ark {

DrawDecoratorComposite::DrawDecoratorComposite(sp<DrawDecorator> delegate, sp<DrawDecorator> next): _delegate(std::move(delegate)), _next(std::move(next))
{
    DASSERT(_delegate && _next);
}

void DrawDecoratorComposite::preDraw(GraphicsContext& graphicsContext, const DrawingContext& context)
{
    _delegate->preDraw(graphicsContext, context);
    _next->preDraw(graphicsContext, context);
}

void DrawDecoratorComposite::postDraw(GraphicsContext& graphicsContext, const DrawingContext& context)
{
    _next->postDraw(graphicsContext, context);
    _delegate->postDraw(graphicsContext, context);
}

sp<DrawDecorator> DrawDecoratorComposite::compose(sp<DrawDecorator> self, sp<DrawDecorator> next)
{
    if(self && next)
        return sp<DrawDecorator>::make<DrawDecoratorComposite>(std::move(self), std::move(next));
    return std::move(self ? self : next);
}

}
