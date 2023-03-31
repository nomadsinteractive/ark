#pragma once

#include "core/base/wrapper.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderable.h"

namespace ark {

class RenderableNone : public Renderable {
public:

    virtual StateBits updateState(const RenderRequest& renderRequest) override;
    virtual Snapshot snapshot(const PipelineInput& pipelineInput, const RenderRequest& renderRequest, const V3& postTranslate, StateBits state) override;

};

}
