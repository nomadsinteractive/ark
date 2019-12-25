#ifndef ARK_GRAPHICS_IMPL_RENDERABLE_RENDERABLE_PASSIVE_H_
#define ARK_GRAPHICS_IMPL_RENDERABLE_RENDERABLE_PASSIVE_H_

#include "graphics/forwarding.h"
#include "graphics/inf/renderable.h"

namespace ark {

class RenderablePassive : public Renderable {
public:
    RenderablePassive(const sp<RenderObject>& renderObject = nullptr);

    virtual Snapshot snapshot(const PipelineInput& pipelineInput, const RenderRequest& renderRequest) override;

    const sp<RenderObject>& renderObject() const;

    void requestUpdate(const V3& position);

private:
    sp<RenderObject> _render_object;
    bool _position_changed;
    bool _render_requested;
    bool _visible;
    V3 _position;
};

}

#endif
