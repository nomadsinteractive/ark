#ifndef ARK_GRAPHICS_IMPL_RENDERER_RENDERER_WITH_EXPIRABLE_H_
#define ARK_GRAPHICS_IMPL_RENDERER_RENDERER_WITH_EXPIRABLE_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class RendererWithExpired {
public:

//  [[plugin::style("expired")]]
    class DECORATOR : public Builder<Renderer> {
    public:
        DECORATOR(BeanFactory& factory, const sp<Builder<Renderer>>& delegate, const String& value);

        virtual sp<Renderer> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<Disposed>> _expired;
    };

};

}

#endif
