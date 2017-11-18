#ifndef ARK_APP_IMPL_RENDERER_RENDERER_WITH_STATE_H_
#define ARK_APP_IMPL_RENDERER_RENDERER_WITH_STATE_H_

#include <map>

#include "core/types/shared_ptr.h"

#include "graphics/inf/renderer.h"

#include "app/view/view.h"

namespace ark {

class RendererWithState : public Renderer {
public:
    RendererWithState(const sp<Renderer>& def);

    virtual void render(RenderRequest& renderRequest, float x, float y) override;

    View::State status() const;
    void setStatus(View::State status);

    void setStateRenderer(View::State status, const sp<Renderer>& drawable);

    const sp<Renderer>& getRendererByCurrentStatus();

private:
    View::State _state;
    sp<Renderer> _default;
    std::map<View::State, sp<Renderer>> _selector;

};

}

#endif
