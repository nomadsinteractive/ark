#pragma once

#include <vector>

#include "core/base/api.h"
#include "core/types/shared_ptr.h"
#include "core/types/optional.h"

#include "graphics/base/render_layer_snapshot.h"
#include "graphics/base/rect.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/pipeline_context.h"
#include "renderer/base/drawing_context_params.h"

namespace ark {

class ARK_API DrawingContext : public PipelineContext {
public:
    DrawingContext(sp<ShaderBindings> shaderBindings, sp<ByType> attachments, std::vector<RenderLayerSnapshot::UBOSnapshot> ubo, std::vector<std::pair<uint32_t, Buffer::Snapshot>> ssbos);
    DrawingContext(sp<ShaderBindings> shaderBindings, sp<ByType> attachments, std::vector<RenderLayerSnapshot::UBOSnapshot> ubo, std::vector<std::pair<uint32_t, Buffer::Snapshot>> ssbos, Buffer::Snapshot vertexBuffer, Buffer::Snapshot indexBuffer, DrawingContextParams::Parameters parameters);
    DEFAULT_COPY_AND_ASSIGN(DrawingContext);

    sp<RenderCommand> toRenderCommand(const RenderRequest& renderRequest);
    sp<RenderCommand> toBindCommand();

    void upload(GraphicsContext& graphicsContext);

    sp<ByType> _attachments;
    Optional<Rect> _scissor;

    Buffer::Snapshot _vertices;
    Buffer::Snapshot _indices;

    DrawingContextParams::Parameters _parameters;

};

}
