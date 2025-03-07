#pragma once

#include "core/base/api.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API DrawingContextComposer {
public:
    virtual ~DrawingContextComposer() = default;

    virtual sp<PipelineBindings> makePipelineBindings(const Shader& shader, RenderController& renderController, Enum::DrawMode renderMode) = 0;
    virtual DrawingContext compose(const RenderRequest& renderRequest, const RenderLayerSnapshot& snapshot) = 0;
};

}
