#ifndef ARK_APP_IMPL_RENDERER_RENDERER_WITH_STATE_H_
#define ARK_APP_IMPL_RENDERER_RENDERER_WITH_STATE_H_

#include <map>

#include "core/types/shared_ptr.h"

#include "graphics/inf/renderer.h"

#include "app/view/view.h"

namespace ark {

class RendererWithState : public Renderer {
public:
    RendererWithState(const sp<Renderer>& def, const sp<View::State>& state);

    virtual void render(RenderRequest& renderRequest, float x, float y) override;

    void setStateRenderer(View::State status, sp<Renderer> renderer, sp<Boolean> enabled);

    const sp<Renderer>& getRendererByCurrentStatus();

private:
    struct StateRenderer {
        StateRenderer() = default;
        StateRenderer(sp<Renderer> renderer, sp<Boolean> boolean);

        sp<Renderer> _renderer;
        sp<Boolean> _enabled;
    };

private:
    sp<Renderer> _default;
    sp<View::State> _state;

    StateRenderer _selectors[View::STATE_COUNT - 1];
};

}

#endif
