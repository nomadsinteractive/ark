#pragma once

#include "core/types/shared_ptr.h"

#include "renderer/inf/draw_decorator.h"

namespace ark {

class DrawDecoratorFactoryComposite final : public DrawDecoratorFactory {
public:
    DrawDecoratorFactoryComposite(sp<DrawDecoratorFactory> delegate, sp<DrawDecoratorFactory> next);

    sp<DrawDecorator> makeDrawDecorator(const RenderRequest& renderRequest) override;

    static sp<DrawDecoratorFactory> compose(sp<DrawDecoratorFactory> self, sp<DrawDecoratorFactory> next);

private:
    sp<DrawDecoratorFactory> _delegate;
    sp<DrawDecoratorFactory> _next;
};

}
