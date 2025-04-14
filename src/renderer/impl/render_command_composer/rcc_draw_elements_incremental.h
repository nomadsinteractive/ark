#pragma once

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/graphics_buffer_allocator.h"
#include "renderer/inf/drawing_context_composer.h"

namespace ark {

class RCCDrawElementsIncremental final : public DrawingContextComposer {
public:
    RCCDrawElementsIncremental() = default;

    sp<PipelineBindings> makePipelineBindings(const Shader& shader, RenderController& renderController, enums::DrawMode renderMode) override;
    DrawingContext compose(const RenderRequest& renderRequest, const RenderLayerSnapshot& snapshot) override;

private:
    Buffer _indices;
    sp<GraphicsBufferAllocator::Strips> _strips;
};

}
