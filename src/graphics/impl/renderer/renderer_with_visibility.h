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
    RendererWithVisibility(const sp<Renderer>& renderer, const sp<Visibility>& visibility);

    virtual void render(RenderRequest& renderRequest, float x, float y) override;

//  [[plugin::style("visibility")]]
    class STYLE : public Builder<Renderer> {
    public:
        STYLE(BeanFactory& factory, const sp<Builder<Renderer>>& delegate, const String& value);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<Visibility>> _visibility;
    };

private:
    sp<Renderer> _renderer;
    sp<Visibility> _visibility;

};

}

#endif
