#include "renderer/impl/draw_decorator/draw_decorator_factory_composite.h"

#include "renderer/impl/draw_decorator/draw_decorator_composite.h"

namespace ark {

DrawDecoratorFactoryComposite::DrawDecoratorFactoryComposite(sp<DrawDecoratorFactory> delegate, sp<DrawDecoratorFactory> next)
    : _delegate(std::move(delegate)), _next(std::move(next))
{
}

sp<DrawDecorator> DrawDecoratorFactoryComposite::makeDrawDecorator(const RenderRequest& renderRequest)
{
    return DrawDecoratorComposite::compose(_delegate->makeDrawDecorator(renderRequest), _next->makeDrawDecorator(renderRequest));
}

sp<DrawDecoratorFactory> DrawDecoratorFactoryComposite::compose(sp<DrawDecoratorFactory> self, sp<DrawDecoratorFactory> next)
{
    if(self && next)
        return sp<DrawDecoratorFactory>::make<DrawDecoratorFactoryComposite>(std::move(self), std::move(next));
    return std::move(self ? self : next);
}

}
