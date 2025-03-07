#pragma once

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/base/enum.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/inf/snippet.h"

namespace ark {

class ARK_API PipelineBindings {
public:
    PipelineBindings(Enum::DrawMode drawMode, Enum::DrawProcedure drawProcedure, Buffer vertices, sp<PipelineDescriptor> pipelineDescriptor, Map<uint32_t, Buffer> streams);

    Enum::DrawMode drawMode() const;
    Enum::DrawProcedure drawProcedure() const;

    const Buffer& vertices() const;

    const sp<PipelineDescriptor>& pipelineDescriptor() const;
    const sp<PipelineLayout>& pipelineLayout() const;

    const sp<Snippet>& snippet() const;

    const sp<Map<uint32_t, Buffer>>& streams() const;
    const sp<Traits>& attachments() const;

    const sp<Pipeline>& ensurePipeline(GraphicsContext& graphicsContext);
    const sp<Pipeline>& ensureRenderPipeline(GraphicsContext& graphicsContext);

    Map<uint32_t, Buffer::Factory> makeDividedBufferFactories() const;

private:
    void doEnsurePipeline(GraphicsContext& graphicsContext);

private:
    Enum::DrawMode _draw_mode;
    Enum::DrawProcedure _draw_procedure;

    Buffer _vertices;
    sp<PipelineDescriptor> _pipeline_descriptor;

    sp<Map<uint32_t, Buffer>> _streams;

    sp<Pipeline> _pipeline;

    sp<Traits> _attachments;
};

}
