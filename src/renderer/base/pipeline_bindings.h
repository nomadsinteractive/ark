#pragma once

#include <map>
#include <vector>

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/inf/snippet.h"

namespace ark {

class ARK_API PipelineBindings {
public:
    PipelineBindings(Buffer vertices, sp<PipelineFactory> pipelineFactory, sp<PipelineDescriptor> pipelineDescriptor, Map<uint32_t, Buffer> streams);

    const Buffer& vertices() const;
    Buffer& vertices();

    const sp<PipelineFactory>& pipelineFactory() const;
    const sp<PipelineDescriptor>& pipelineDescriptor() const;

    const sp<Snippet>& snippet() const;
    void addSnippet(sp<Snippet> snippet);

    const sp<PipelineLayout>& pipelineLayout() const;
    const sp<ShaderLayout>& pipelineInput() const;

    const sp<Map<uint32_t, Buffer>>& streams() const;
    const sp<Traits>& attachments() const;

    const sp<Pipeline>& ensureRenderPipeline(GraphicsContext& graphicsContext);
    const sp<Pipeline>& ensureComputePipeline(GraphicsContext& graphicsContext);

    Map<uint32_t, Buffer::Factory> makeDividedBufferFactories() const;

private:
    void doEnsurePipeline(GraphicsContext& graphicsContext);

private:
    Buffer _vertices;
    sp<PipelineFactory> _pipeline_factory;
    sp<PipelineDescriptor> _pipeline_descriptor;
    sp<Snippet> _snippet;

    sp<Map<uint32_t, Buffer>> _streams;

    sp<Pipeline> _render_pipeline;
    sp<Pipeline> _compute_pipeline;

    sp<Traits> _attachments;
};

}
