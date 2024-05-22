#pragma once

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"

namespace ark {

class RendererStylePosition : public Renderer {
public:
    RendererStylePosition(sp<Renderer> renderer, sp<Vec3> position);

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
