#pragma once

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/base/enum.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/inf/snippet.h"

namespace ark {

class ARK_API PipelineBindings {
public:
    PipelineBindings(Enum::DrawMode drawMode, Enum::DrawProcedure drawProcedure, Buffer vertices, sp<PipelineDescriptor> pipelineDescriptor, Vector<std::pair<uint32_t, Buffer>> instanceBuffers = {});
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(PipelineBindings);

    Enum::DrawMode drawMode() const;
    Enum::DrawProcedure drawProcedure() const;

    const Buffer& vertices() const;

    const sp<PipelineDescriptor>& pipelineDescriptor() const;
    const sp<PipelineLayout>& pipelineLayout() const;

    const sp<Snippet>& snippet() const;

    const sp<Traits>& attachments() const;

    const Vector<std::pair<uint32_t, Buffer>>& instanceBuffers() const;
    const Vector<std::pair<sp<Texture>, PipelineLayout::DescriptorSet>>& samplers() const;
    const Vector<std::pair<sp<Texture>, PipelineLayout::DescriptorSet>>& images() const;

    const sp<Pipeline>& ensurePipeline(GraphicsContext& graphicsContext);
    const sp<Pipeline>& ensureRenderPipeline(GraphicsContext& graphicsContext);
    const sp<Pipeline>& ensureComputePipeline(GraphicsContext& graphicsContext);

    void bindSampler(sp<Texture> texture, uint32_t name = 0) const;

    Vector<std::pair<uint32_t, Buffer::Snapshot>> makeInstanceBufferSnapshots() const;
    Map<uint32_t, Buffer::SnapshotFactory> makeDividedBufferFactories() const;

private:
    void doEnsurePipeline(GraphicsContext& graphicsContext);

private:
    struct Stub;
    sp<Stub> _stub;
    sp<Pipeline> _render_pipeline;
    sp<Pipeline> _compute_pipeline;
    sp<Pipeline> _pipeline;
};

}
