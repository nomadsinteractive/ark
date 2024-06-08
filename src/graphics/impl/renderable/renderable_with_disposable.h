#pragma once

#include "core/base/wrapper.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderable.h"

namespace ark {

class RenderableWithDiscarded final : public Wrapper<Renderable>, public Renderable {
public:
    RenderableWithDiscarded(sp<Renderable> delegate, sp<Boolean> disposed);

    StateBits updateState(const RenderRequest& renderRequest) override;
    Snapshot snapshot(const LayerContextSnapshot& snapshotContext, const RenderRequest& renderRequest, StateBits state) override;

private:
    sp<Boolean> _discarded;
};

}
