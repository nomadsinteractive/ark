#pragma once

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/graphics_buffer_allocator.h"
#include "renderer/inf/render_command_composer.h"

namespace ark {

class RCCDrawElementsIncremental final : public RenderCommandComposer {
public:
    RCCDrawElementsIncremental() = default;

    sp<PipelineBindings> makeShaderBindings(Shader& shader, RenderController& renderController, Enum::RenderMode renderMode) override;
    sp<RenderCommand> compose(const RenderRequest& renderRequest, const RenderLayerSnapshot& snapshot) override;

private:
    Buffer _indices;
    sp<GraphicsBufferAllocator::Strips> _strips;
};

}
