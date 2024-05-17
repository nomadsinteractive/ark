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

class ARK_API ShaderBindings {
public:
    ShaderBindings(Buffer vertices, sp<PipelineFactory> pipelineFactory, sp<PipelineBindings> pipelineBindings, std::map<uint32_t, Buffer> dividedBuffers);

    const Buffer& vertices() const;
    Buffer& vertices();

    const sp<PipelineFactory>& pipelineFactory() const;
    const sp<PipelineBindings>& pipelineBindings() const;

    const sp<Snippet>& snippet() const;
    void addSnippet(sp<Snippet> snippet);

    const sp<PipelineLayout>& pipelineLayout() const;
    const sp<PipelineInput>& pipelineInput() const;
    const std::vector<sp<Texture>>& samplers() const;
    const sp<std::map<uint32_t, Buffer>>& divisors() const;
    const sp<Traits>& attachments() const;

    const sp<Pipeline>& getPipeline(GraphicsContext& graphicsContext);

    std::map<uint32_t, Buffer::Factory> makeDividedBufferFactories() const;

private:
    Buffer _vertices;
    sp<PipelineFactory> _pipeline_factory;
    sp<PipelineBindings> _pipeline_bindings;
    sp<Snippet> _snippet;

    sp<std::map<uint32_t, Buffer>> _divided_buffers;

    sp<Pipeline> _pipeline;

    sp<Traits> _attachments;

};

}
