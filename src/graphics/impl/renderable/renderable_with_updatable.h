#pragma once

#include "core/base/wrapper.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderable.h"

namespace ark {

class RenderableWithUpdatable final : public Wrapper<Renderable>, public Renderable {
public:
    RenderableWithUpdatable(sp<Renderable> delegate, sp<Updatable> updatable);

    StateBits updateState(const RenderRequest& renderRequest) override;
    Snapshot snapshot(const LayerContextSnapshot& snapshotContext, const RenderRequest& renderRequest, StateBits state) override;

private:
    sp<Updatable> _updatable;
};

}
