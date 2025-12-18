#pragma once

#include "core/types/shared_ptr.h"

#include "renderer/inf/draw_decorator.h"

namespace ark {

class DrawDecoratorFactoryCompound final : public DrawDecoratorFactory {
public:
    DrawDecoratorFactoryCompound(sp<DrawDecoratorFactory> delegate, sp<DrawDecoratorFactory> next);

    sp<DrawDecorator> makeDrawDecorator(const RenderRequest& renderRequest) override;

    static sp<DrawDecoratorFactory> compound(sp<DrawDecoratorFactory> self, sp<DrawDecoratorFactory> next);

private:
    sp<DrawDecoratorFactory> _delegate;
    sp<DrawDecoratorFactory> _next;
};

}
