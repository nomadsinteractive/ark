#ifndef ARK_GRAPHICS_IMPL_RENDERER_RENDERER_STYLE_BOUNDS_H_
#define ARK_GRAPHICS_IMPL_RENDERER_RENDERER_STYLE_BOUNDS_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"

namespace ark {

class RendererStyleBounds : public Renderer {
public:
    RendererStyleBounds(const sp<Renderer>& renderer, const sp<Vec4>& bounds);

    virtual void render(RenderRequest& renderRequest, const V3& position) override;

//  [[plugin::style("bounds")]]
    class STYLE : public Builder<Renderer> {
    public:
        STYLE(BeanFactory& factory, const sp<Builder<Renderer>>& delegate, const String& value);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<Vec4>> _bounds;
    };

private:
    sp<Renderer> _renderer;
    sp<Vec4Impl> _bounds;
    sp<Size> _size;
};

}

#endif
