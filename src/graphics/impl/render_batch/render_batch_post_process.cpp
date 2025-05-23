#include "render_batch_post_process.h"

#include "core/base/named_hash.h"
#include "core/types/global.h"

#include "graphics/base/layer_context.h"
#include "graphics/components/render_object.h"
#include "graphics/util/renderable_type.h"
#include "graphics/util/vec3_type.h"

#include "renderer/impl/model_loader/model_loader_ndc.h"
#include "renderer/base/render_engine.h"

#include "app/base/application_context.h"
#include "graphics/util/renderable_type.h"

namespace ark {

RenderBatchPostProcess::RenderBatchPostProcess()
    : RenderBatch(Global<Constants>()->BOOLEAN_FALSE), _layer_context(sp<LayerContext>::make(nullptr, sp<ModelLoader>::make<ModelLoaderNDC>(), nullptr, nullptr, Global<Constants>()->BOOLEAN_FALSE))
{
    const RenderEngine& renderEngine = Ark::instance().applicationContext()->renderEngine();
    sp<Vec3> position = Vec3Type::create(renderEngine.viewport().width() / 2, renderEngine.viewport().height() / 2, 0);
    sp<Size> size = sp<Size>::make(renderEngine.viewport().width(), renderEngine.viewport().height());
    sp<Renderable> renderable = RenderableType::create(sp<Renderable>::make<RenderObject>(NamedHash(1), std::move(position), std::move(size)), nullptr, Global<Constants>()->BOOLEAN_FALSE);
    _layer_context->pushBack(std::move(renderable));
    _contexts.push_back(_layer_context);
}

Vector<sp<LayerContext>>& RenderBatchPostProcess::snapshot(const RenderRequest& renderRequest)
{
    return _contexts;
}

}
