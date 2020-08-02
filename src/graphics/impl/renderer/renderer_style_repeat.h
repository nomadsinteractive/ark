#ifndef ARK_GRAPHICS_IMPL_RENDERER_RENDERER_STYLE_REPEAT_H_
#define ARK_GRAPHICS_IMPL_RENDERER_RENDERER_STYLE_REPEAT_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"

namespace ark {

class RendererStyleRepeat : public Renderer {
public:
    RendererStyleRepeat(sp<Renderer> delegate, sp<Integer> repeat);

    virtual void render(RenderRequest& renderRequest, const V3& position) override;

//  [[plugin::style("repeat")]]
    class STYLE : public Builder<Renderer> {
    public:
        STYLE(BeanFactory& factory, const sp<Builder<Renderer>>& delegate, const String& value);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<Integer>> _repeat;
    };

private:
    sp<Renderer> _delegate;
    sp<Integer> _repeat;

};

}

#endif
