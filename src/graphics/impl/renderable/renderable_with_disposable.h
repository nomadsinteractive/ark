#pragma once

#include "core/base/wrapper.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderable.h"

namespace ark {

class RenderableWithDisposable : public Wrapper<Renderable>, public Renderable {
public:
    RenderableWithDisposable(sp<Renderable> delegate, sp<Boolean> disposed);

    virtual StateBits updateState(const RenderRequest& renderRequest) override;
    virtual Snapshot snapshot(const LayerContextSnapshot& snapshotContext, const RenderRequest& renderRequest, const V3& postTranslate, StateBits state) override;

private:
    sp<Boolean> _discarded;
};

}
