#include "renderer/impl/draw_decorator/draw_decorator_factory_compound.h"

#include "renderer/impl/draw_decorator/draw_decorator_composite.h"

namespace ark {

DrawDecoratorFactoryCompound::DrawDecoratorFactoryCompound(sp<DrawDecoratorFactory> delegate, sp<DrawDecoratorFactory> next)
    : _delegate(std::move(delegate)), _next(std::move(next))
{
}

sp<DrawDecorator> DrawDecoratorFactoryCompound::makeDrawDecorator(const RenderRequest& renderRequest)
{
    return DrawDecoratorComposite::compose(_delegate->makeDrawDecorator(renderRequest), _next->makeDrawDecorator(renderRequest));
}

sp<DrawDecoratorFactory> DrawDecoratorFactoryCompound::compound(sp<DrawDecoratorFactory> self, sp<DrawDecoratorFactory> next)
{
    if(self && next)
        return sp<DrawDecoratorFactory>::make<DrawDecoratorFactoryCompound>(std::move(self), std::move(next));
    return std::move(self ? self : next);
}

}
