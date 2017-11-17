#ifndef ARK_GRAPHICS_IMPL_RENDERER_RENDERER_WITH_VISIBILITY_H_
#define ARK_GRAPHICS_IMPL_RENDERER_RENDERER_WITH_VISIBILITY_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"

namespace ark {

class RendererWithVisibility : public Renderer {
public:
    RendererWithVisibility(const sp<Renderer>& renderer, const sp<Boolean>& visibility);

    virtual void render(RenderRequest& renderRequest, float x, float y) override;

//  [[plugin::style("visibility")]]
    class DECORATOR : public Builder<Renderer> {
    public:
        DECORATOR(BeanFactory& parent, const sp<Builder<Renderer>>& delegate, const String& value);

        virtual sp<Renderer> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<Boolean>> _visibility;
    };

private:
    sp<Renderer> _renderer;
    sp<Boolean> _visibility;

};

}

#endif
