#pragma once

#include "core/inf/builder.h"

#include "graphics/inf/render_batch.h"

namespace ark {

class RenderBatchPostProcess final : public RenderBatch {
public:
    RenderBatchPostProcess();

    std::vector<sp<LayerContext>>& snapshot(const RenderRequest& renderRequest) override;

//  [[plugin::resource-loader("post-process")]]
    class BUILDER final : public Builder<RenderBatch> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<RenderBatch> build(const Scope& args) override;

    };

private:
    std::vector<sp<LayerContext>> _contexts;

    sp<LayerContext> _layer_context;
};

}
