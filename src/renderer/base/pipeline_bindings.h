#ifndef ARK_RENDERER_BASE_PIPELINE_BINDINGS_H_
#define ARK_RENDERER_BASE_PIPELINE_BINDINGS_H_

#include "core/base/api.h"

#include "renderer/inf/render_model.h"

namespace ark {

class ARK_API PipelineBindings {
public:
    enum AttributeName {
        ATTRIBUTE_NAME_TEX_COORDINATE,
        ATTRIBUTE_NAME_NORMAL,
        ATTRIBUTE_NAME_TANGENT,
        ATTRIBUTE_NAME_BITANGENT,
        ATTRIBUTE_NAME_COUNT
    };

    enum Flag {
        FLAG_CULL_MODE_NONE = 0,
        FLAG_CULL_MODE_CW = 1,
        FLAG_CULL_MODE_CCW = 2,
        FLAG_CULL_MODE_BITMASK = 3,
        FLAG_DYNAMIC_SCISSOR = 4,
        FLAG_DYNAMIC_SCISSOR_BITMASK = 4,
        FLAG_DEFAULT_VALUE = FLAG_CULL_MODE_CCW
    };

    struct Attributes {
        Attributes(const PipelineInput& input);

        int32_t _offsets[ATTRIBUTE_NAME_COUNT];
    };

public:
    PipelineBindings(RenderModel::Mode mode, const sp<PipelineLayout>& pipelineLayout);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(PipelineBindings);

    RenderModel::Mode mode() const;

    const sp<PipelineLayout>& layout() const;
    const sp<PipelineInput>& input() const;
    const sp<Snippet>& snippet() const;

    const Attributes& attributes() const;
    const std::vector<sp<Texture>>& samplers() const;

    void bindSampler(const sp<Texture>& texture, uint32_t name = 0);

    bool hasDivisors() const;

    Flag getFlag(Flag bitmask) const;
    bool hasFlag(Flag flag, Flag bitmask) const;
    void setFlag(Flag flag, Flag bitmask) const;

    sp<Pipeline> getPipeline(GraphicsContext& graphicsContext, const sp<PipelineFactory>& pipelineFactory);

private:
    struct Stub {
        Stub(RenderModel::Mode mode, const sp<PipelineLayout>& pipelineLayout);

        RenderModel::Mode _mode;
        sp<PipelineLayout> _layout;
        sp<PipelineInput> _input;

        Attributes _attributes;

        std::vector<sp<Texture>> _samplers;

        uint32_t _flags;
    };

private:
    sp<Stub> _stub;

    sp<Pipeline> _pipeline;
};

}

#endif
