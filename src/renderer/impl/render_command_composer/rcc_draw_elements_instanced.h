#pragma once

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/model.h"
#include "renderer/inf/render_command_composer.h"

namespace ark {

class RCCDrawElementsInstanced : public RenderCommandComposer {
public:
    RCCDrawElementsInstanced(Model model);

    virtual sp<PipelineBindings> makeShaderBindings(Shader& shader, RenderController& renderController, Enum::RenderMode renderMode) override;

    virtual void postSnapshot(RenderController& renderController, RenderLayerSnapshot& snapshot) override;
    virtual sp<RenderCommand> compose(const RenderRequest& renderRequest, RenderLayerSnapshot& snapshot) override;

private:
    Model _model;
    Buffer _indices;
};

}
