#pragma once

#include "graphics/inf/renderable.h"

namespace ark {

class RenderableWithTransform : public Wrapper<Renderable>, public Renderable {
public:
    RenderableWithTransform(sp<Renderable> delegate, sp<Mat4> transform);

    StateBits updateState(const RenderRequest& renderRequest) override;
    Snapshot snapshot(const LayerContextSnapshot& snapshotContext, const RenderRequest& renderRequest, StateBits state) override;

private:
    sp<Mat4> _transform;
};

}
