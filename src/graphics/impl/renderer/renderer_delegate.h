#ifndef ARK_GRAPHICS_IMPL_RENDERER_RENDERER_DELEGATE_H_
#define ARK_GRAPHICS_IMPL_RENDERER_RENDERER_DELEGATE_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/renderer.h"

namespace ark {

class ARK_API RendererDelegate : public Renderer, public Renderer::Delegate {
public:
    RendererDelegate() = default;
    RendererDelegate(const sp<Renderer>& delegate);

    virtual void render(RenderRequest& renderRequest, float x, float y) override;

    virtual void setRendererDelegate(const sp<Renderer>& delegate) override;

    const sp<Renderer>& delegate() const;

private:
    sp<Renderer> _delegate;

};

}

#endif
