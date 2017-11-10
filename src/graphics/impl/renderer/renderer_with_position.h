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
    RendererWithPosition(const sp<Renderer>& renderer, const sp<VV>& position);

    virtual void render(RenderCommandPipeline& pipeline, float x, float y) override;

//  [[plugin::style("position")]]
    class DECORATOR : public Builder<Renderer> {
    public:
        DECORATOR(BeanFactory& parent, const sp<Builder<Renderer>>& delegate, const String& value);

        virtual sp<Renderer> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<VV>> _position;
    };

private:
    sp<Renderer> _renderer;
    sp<VV> _position;

};

}

#endif
