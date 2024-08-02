#pragma once

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/model.h"
#include "renderer/inf/render_command_composer.h"

namespace ark {

class RCCDrawElementsInstanced final : public RenderCommandComposer {
public:
    RCCDrawElementsInstanced(Model model);

    sp<PipelineBindings> makeShaderBindings(Shader& shader, RenderController& renderController, Enum::RenderMode renderMode) override;
    sp<RenderCommand> compose(const RenderRequest& renderRequest, RenderLayerSnapshot& snapshot) override;

private:
    Model _model;
    Buffer _indices;
};

}
