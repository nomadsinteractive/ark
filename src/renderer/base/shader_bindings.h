#ifndef ARK_RENDERER_BASE_SHADER_BINDINGS_H_
#define ARK_RENDERER_BASE_SHADER_BINDINGS_H_

#include <map>
#include <vector>

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"

namespace ark {

class ARK_API ShaderBindings {
public:
    ShaderBindings(const sp<PipelineFactory>& pipelineFactory, const sp<PipelineBindings>& pipelineBindings, RenderController& renderController);

    const sp<PipelineFactory>& pipelineFactory() const;
    const sp<PipelineBindings>& pipelineBindings() const;

    const sp<Snippet>& snippet() const;
    const sp<PipelineLayout>& pipelineLayout() const;
    const sp<PipelineInput>& pipelineInput() const;
    const std::vector<sp<Texture>>& samplers() const;
    const sp<std::map<uint32_t, Buffer>>& divisors() const;

    sp<Pipeline> getPipeline(GraphicsContext& graphicsContext);

    std::map<uint32_t, Buffer::Builder> makeDividedBufferBuilders() const;

private:
    sp<std::map<uint32_t, Buffer>> makeDivisors(RenderController& renderController) const;

private:
    sp<PipelineFactory> _pipeline_factory;
    sp<PipelineBindings> _pipeline_bindings;

    sp<std::map<uint32_t, Buffer>> _divisors;

    sp<Pipeline> _pipeline;
};

}

#endif
