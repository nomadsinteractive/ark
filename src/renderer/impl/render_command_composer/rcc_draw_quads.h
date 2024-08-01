#pragma once

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/graphics_buffer_allocator.h"
#include "renderer/inf/render_command_composer.h"

namespace ark {

class RCCDrawQuads : public RenderCommandComposer {
public:
    RCCDrawQuads(sp<Model> model);

    sp<PipelineBindings> makeShaderBindings(Shader& shader, RenderController& renderController, Enum::RenderMode renderMode) override;

    void postSnapshot(RenderController& renderController, RenderLayerSnapshot& snapshot) override;
    sp<RenderCommand> compose(const RenderRequest& renderRequest, RenderLayerSnapshot& snapshot) override;

private:
    sp<Model> _model;
    Buffer _indices;
    sp<GraphicsBufferAllocator::Strips> _strips;
};

}
