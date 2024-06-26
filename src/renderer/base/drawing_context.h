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
#include "renderer/base/drawing_params.h"

namespace ark {

class ARK_API DrawingContext : public PipelineContext {
public:
    DrawingContext(sp<ShaderBindings> shaderBindings, sp<Traits> attachments, std::vector<RenderLayerSnapshot::UBOSnapshot> ubo, std::vector<std::pair<uint32_t, Buffer::Snapshot>> ssbos);
    DrawingContext(sp<ShaderBindings> shaderBindings, sp<Traits> attachments, std::vector<RenderLayerSnapshot::UBOSnapshot> ubo, std::vector<std::pair<uint32_t, Buffer::Snapshot>> ssbos,
                   Buffer::Snapshot vertexBuffer, Buffer::Snapshot indexBuffer, uint32_t drawCount, DrawingParams parameters);
    DEFAULT_COPY_AND_ASSIGN(DrawingContext);

    sp<RenderCommand> toRenderCommand(const RenderRequest& renderRequest);
    sp<RenderCommand> toBindCommand();

    void upload(GraphicsContext& graphicsContext);

    sp<Traits> _attachments;
    Optional<Rect> _scissor;

    Buffer::Snapshot _vertices;
    Buffer::Snapshot _indices;

    uint32_t _draw_count;
    DrawingParams _parameters;

};

}
