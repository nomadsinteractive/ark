#pragma once


#include "renderer/forwarding.h"
#include "renderer/base/render_controller.h"
#include "renderer/inf/render_command_composer.h"

namespace ark {

class RCCDrawElements final : public RenderCommandComposer {
public:
    RCCDrawElements(sp<Model> model);

    sp<PipelineBindings> makePipelineBindings(const Shader& shader, RenderController& renderController, Enum::RenderMode renderMode) override;
    DrawingContext compose(const RenderRequest& renderRequest, const RenderLayerSnapshot& snapshot) override;

private:
    sp<Model> _model;
    sp<RenderController::PrimitiveIndexBuffer> _primitive_index_buffer;
};

}
