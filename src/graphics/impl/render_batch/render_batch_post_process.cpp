#include "render_batch_post_process.h"

#include "renderer/impl/model_loader/model_loader_unit_quad.h"

namespace ark {

RenderBatchPostProcess::RenderBatchPostProcess()
    : _layer_context(sp<LayerContext>::make(nullptr, sp<ModelLoader>::make<ModelLoaderUnitQuad>()))
{
}

sp<RenderBatch> RenderBatchPostProcess::BUILDER::build(const Scope& args)
{
    return sp<RenderBatch>::make<RenderBatchPostProcess>();
}

}
