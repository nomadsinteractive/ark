#pragma once

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/model.h"
#include "renderer/inf/drawing_context_composer.h"

namespace ark {

class RCCDrawElementsInstanced final : public DrawingContextComposer {
public:
    RCCDrawElementsInstanced(Model model);

    sp<PipelineBindings> makePipelineBindings(const Shader& shader, RenderController& renderController, Enum::DrawMode renderMode) override;
    DrawingContext compose(const RenderRequest& renderRequest, const RenderLayerSnapshot& snapshot) override;

private:
    Model _model;
    Buffer _indices;
};

}
