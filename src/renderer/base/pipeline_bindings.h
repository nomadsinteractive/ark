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
    PipelineBindings(Enum::DrawMode drawMode, Enum::DrawProcedure drawProcedure, Buffer vertices, sp<PipelineDescriptor> pipelineDescriptor, Map<uint32_t, Buffer> streams);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(PipelineBindings);

    Enum::DrawMode drawMode() const;
    Enum::DrawProcedure drawProcedure() const;

    const Buffer& vertices() const;

    const sp<PipelineDescriptor>& pipelineDescriptor() const;
    const sp<PipelineLayout>& pipelineLayout() const;

    const sp<Snippet>& snippet() const;

    const Map<uint32_t, Buffer>& streams() const;
    const sp<Traits>& attachments() const;

    const Vector<std::pair<sp<Texture>, PipelineLayout::DescriptorSet>>& samplers() const;
    const Vector<std::pair<sp<Texture>, PipelineLayout::DescriptorSet>>& images() const;

    const sp<Pipeline>& ensurePipeline(GraphicsContext& graphicsContext);
    const sp<Pipeline>& ensureRenderPipeline(GraphicsContext& graphicsContext);

    void bindSampler(sp<Texture> texture, uint32_t name = 0) const;

    Map<uint32_t, Buffer::Factory> makeDividedBufferFactories() const;

private:
    void doEnsurePipeline(GraphicsContext& graphicsContext);

private:
    struct Stub;
    sp<Stub> _stub;
    sp<Pipeline> _pipeline;
};

}
