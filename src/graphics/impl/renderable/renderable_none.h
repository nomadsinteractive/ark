#pragma once

#include "core/base/wrapper.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderable.h"

namespace ark {

class RenderableNone : public Renderable {
public:

    StateBits updateState(const RenderRequest& renderRequest) override;
    Snapshot snapshot(const LayerContextSnapshot& snapshotContext, const RenderRequest& renderRequest, const V3& postTranslate, StateBits state) override;

};

}
