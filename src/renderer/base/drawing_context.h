#pragma once

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
    DrawingContext(sp<PipelineBindings> pipelineBindings, sp<RenderLayerSnapshot::BufferObject> bufferObject, Buffer::Snapshot vertices = {}, Buffer::Snapshot indices = {}, uint32_t drawCount = 0,
                   DrawingParams parameters = {}, const sp<Traits>& attachments = nullptr);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(DrawingContext);

    sp<RenderCommand> toRenderCommand(const RenderRequest& renderRequest);
    sp<RenderCommand> toNoopCommand(const RenderRequest& renderRequest);

    void upload(GraphicsContext& graphicsContext) const;

    Buffer::Snapshot _vertices;
    Buffer::Snapshot _indices;

    uint32_t _draw_count;
    DrawingParams _parameters;
    sp<Traits> _attachments;

    Optional<Rect> _scissor;
};

}
