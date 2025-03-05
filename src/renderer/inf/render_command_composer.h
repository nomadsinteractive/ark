#pragma once

#include "core/base/api.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API RenderCommandComposer {
public:
    virtual ~RenderCommandComposer() = default;

    virtual sp<PipelineBindings> makePipelineBindings(const Shader& shader, RenderController& renderController, Enum::RenderMode renderMode) = 0;
    virtual DrawingContext compose(const RenderRequest& renderRequest, const RenderLayerSnapshot& snapshot) = 0;
};

}
