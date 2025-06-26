#pragma once

#include "core/base/wrapper.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderable.h"

namespace ark {

class RenderableWithVisible final : public Wrapper<Renderable>, public Renderable {
public:
    RenderableWithVisible(sp<Renderable> delegate, sp<Boolean> visible);

    State updateState(const RenderRequest& renderRequest) override;
    Snapshot snapshot(const LayerContextSnapshot& snapshotContext, const RenderRequest& renderRequest, State state) override;

private:
    sp<Boolean> _visible;
};

}
