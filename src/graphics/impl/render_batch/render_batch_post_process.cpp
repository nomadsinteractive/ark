#include "render_batch_post_process.h"

#include "core/base/named_type.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/render_object.h"
#include "graphics/util/vec3_type.h"

#include "renderer/impl/model_loader/model_loader_unit_quad.h"
#include "renderer/base/render_engine.h"

#include "app/base/application_context.h"

namespace ark {

RenderBatchPostProcess::RenderBatchPostProcess()
    : RenderBatch(Global<Constants>()->BOOLEAN_FALSE), _layer_context(sp<LayerContext>::make(nullptr, sp<ModelLoader>::make<ModelLoaderUnitQuad>(), nullptr, nullptr, Global<Constants>()->BOOLEAN_FALSE))
{
    const RenderEngine& renderEngine = Ark::instance().applicationContext()->renderEngine();
    sp<Vec3> position = Vec3Type::create(renderEngine.viewport().width() / 2, renderEngine.viewport().height() / 2, 0);
    sp<Size> size = sp<Size>::make(renderEngine.viewport().width(), renderEngine.viewport().height());
    _layer_context->add(sp<Renderable>::make<RenderObject>(NamedType(1), std::move(position), std::move(size)), nullptr, Global<Constants>()->BOOLEAN_FALSE);
    _contexts.push_back(_layer_context);
}

std::vector<sp<LayerContext>>& RenderBatchPostProcess::snapshot(const RenderRequest& renderRequest)
{
    return _contexts;
}

}
