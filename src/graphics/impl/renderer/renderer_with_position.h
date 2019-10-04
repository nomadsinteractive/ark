#ifndef ARK_GRAPHICS_IMPL_RENDERER_RENDERER_WITH_POSITION_H_
#define ARK_GRAPHICS_IMPL_RENDERER_RENDERER_WITH_POSITION_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"

namespace ark {

class RendererWithPosition : public Renderer {
public:
    RendererWithPosition(const sp<Renderer>& renderer, const sp<Vec3>& position);

    virtual void render(RenderRequest& renderRequest, const V3& position) override;

//  [[plugin::style("position")]]
    class STYLE : public Builder<Renderer> {
    public:
        STYLE(BeanFactory& factory, const sp<Builder<Renderer>>& delegate, const String& value);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<Vec3>> _position;
    };

private:
    sp<Renderer> _renderer;
    sp<Vec3> _position;

};

}

#endif
