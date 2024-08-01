#pragma once

#include "core/base/api.h"

#include "renderer/forwarding.h"
#include "renderer/inf/model_loader.h"

namespace ark {

class ARK_API RenderCommandComposer {
public:
    virtual ~RenderCommandComposer() = default;

    virtual sp<PipelineBindings> makeShaderBindings(Shader& shader, RenderController& renderController, Enum::RenderMode renderMode) = 0;
    virtual void postSnapshot(RenderController& renderController, RenderLayerSnapshot& snapshot) = 0;

    virtual sp<RenderCommand> compose(const RenderRequest& renderRequest, RenderLayerSnapshot& snapshot) = 0;

};

}
