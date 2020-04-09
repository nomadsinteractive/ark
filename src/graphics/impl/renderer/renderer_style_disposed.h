#ifndef ARK_GRAPHICS_IMPL_RENDERER_RENDERER_WITH_EXPIRABLE_H_
#define ARK_GRAPHICS_IMPL_RENDERER_RENDERER_WITH_EXPIRABLE_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class RendererWithExpired {
public:

//  [[plugin::style("disposed")]]
    class STYLE : public Builder<Renderer> {
    public:
        STYLE(BeanFactory& factory, const sp<Builder<Renderer>>& delegate, const String& value);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<Disposed>> _disposed;
    };

//  [[plugin::style("disposable")]]
    class STYLE_DISPOSABLE : public Builder<Renderer> {
    public:
        STYLE_DISPOSABLE(BeanFactory& factory, const sp<Builder<Renderer>>& delegate, const String& value);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
    };

};

}

#endif
