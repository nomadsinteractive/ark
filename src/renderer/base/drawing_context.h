#pragma once

#include <vector>

#include "core/base/api.h"
#include "core/types/shared_ptr.h"
#include "core/types/optional.h"

#include "graphics/base/render_layer_snapshot.h"
#include "graphics/base/rect.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/pipeline_snapshot.h"
#include "renderer/base/drawing_params.h"

namespace ark {

class ARK_API DrawingContext {
public:
    DrawingContext(PipelineSnapshot pipelineContext, sp<Traits> attachments);
    DrawingContext(PipelineSnapshot pipelineContext, sp<Traits> attachments, Buffer::Snapshot vertices, Buffer::Snapshot indices, uint32_t drawCount, DrawingParams parameters);
    DEFAULT_COPY_AND_ASSIGN(DrawingContext);

    sp<RenderCommand> toRenderCommand(const RenderRequest& renderRequest);
    sp<RenderCommand> toBindCommand();

    void upload(GraphicsContext& graphicsContext) const;

    PipelineSnapshot _pipeline_context;
    sp<Traits> _attachments;

    Buffer::Snapshot _vertices;
    Buffer::Snapshot _indices;

    uint32_t _draw_count;
    DrawingParams _parameters;

    Optional<Rect> _scissor;
};

}
