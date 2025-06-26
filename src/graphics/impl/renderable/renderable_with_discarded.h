#pragma once

#include "core/base/wrapper.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderable.h"

namespace ark {

class RenderableWithDiscarded final : public Wrapper<Renderable>, public Renderable {
public:
    RenderableWithDiscarded(sp<Renderable> delegate, sp<Boolean> discarded);

    State updateState(const RenderRequest& renderRequest) override;
    Snapshot snapshot(const LayerContextSnapshot& snapshotContext, const RenderRequest& renderRequest, State state) override;

private:
    sp<Boolean> _discarded;
};

}
