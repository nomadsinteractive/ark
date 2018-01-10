#ifndef ARK_GRAPHICS_IMPL_RENDERER_RENDERER_DELEGATE_H_
#define ARK_GRAPHICS_IMPL_RENDERER_RENDERER_DELEGATE_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/renderer.h"

namespace ark {

class ARK_API RendererDelegate : public Renderer, public Renderer::Delegate {
public:
    RendererDelegate();
    RendererDelegate(const sp<Renderer>& delegate);

    virtual void render(RenderRequest& renderRequest, float x, float y) override;

    virtual void setRendererDelegate(const sp<Renderer>& delegate) override;

    const sp<Renderer>& delegate() const;

//  [[plugin::builder("renderer-delegate")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& beanFactory, const document& manifest);

        virtual sp<Renderer> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
    };

private:
    sp<Renderer> _delegate;

};

}

#endif
