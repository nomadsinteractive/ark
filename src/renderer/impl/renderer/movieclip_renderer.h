#ifndef ARK_RENDERER_IMPL_RENDERER_MOVIECLIP_RENDERER_H_
#define ARK_RENDERER_IMPL_RENDERER_MOVIECLIP_RENDERER_H_

#include "core/inf/builder.h"
#include "core/epi/expired.h"

#include "graphics/base/frame.h"
#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"

#include "renderer/base/resource_loader_context.h"

namespace ark {

//[[core::class]]
class MovieclipRenderer : public Renderer, public Expired {
public:
    MovieclipRenderer(const sp<Movieclip>& movieclip, const sp<Numeric>& duration, float interval);

    virtual void render(RenderRequest& renderRequest, float x, float y) override;

//  [[plugin::builder("movieclip")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Renderer> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Movieclip>> _movieclip;
        sp<Builder<Numeric>> _duration;
        float _interval;
    };

private:
    sp<Movieclip> _movieclip;
    sp<Numeric> _duration;
    float _interval;

    sp<Renderer> _current;
    uint32_t _frame_id;
};

}

#endif
