#include "graphics/impl/renderer/renderer_by_render_object.h"

#include "core/base/bean_factory.h"
#include "core/types/safe_ptr.h"

#include "graphics/base/layer.h"
#include "graphics/base/layer_context.h"
#include "graphics/base/metrics.h"
#include "graphics/base/render_object.h"
#include "graphics/base/render_layer.h"
#include "graphics/base/size.h"

#include "renderer/inf/render_model.h"

namespace ark {

RendererByRenderObject::RendererByRenderObject(const sp<RenderObject>& renderObject, const sp<LayerContext>& layerContext)
    : _layer_context(layerContext), _renderable(sp<RenderablePassive>::make(renderObject))
{
    DASSERT(_layer_context);
    if(!_renderable->renderObject()->_size)
        measure(_renderable->renderObject()->_size.ensure());
    _layer_context->add(_renderable, renderObject->disposed());
}

void RendererByRenderObject::render(RenderRequest& /*renderRequest*/, const V3& position)
{
    _renderable->requestUpdate(position);
}

const sp<Size>& RendererByRenderObject::size()
{
    return _renderable->renderObject()->_size.ensure();
}

void RendererByRenderObject::measure(Size& size)
{
    const Metrics metrics = _layer_context->renderModel()->measure(_renderable->renderObject()->type()->val());
    size.setWidth(metrics.size.x());
    size.setHeight(metrics.size.y());
}

RendererByRenderObject::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _render_object(factory.ensureBuilder<RenderObject>(manifest)), _layer_context(sp<LayerContext::BUILDER>::make(factory, manifest, Layer::TYPE_UNSPECIFIED))
{
}

sp<Renderer> RendererByRenderObject::BUILDER::build(const Scope& args)
{
    return sp<RendererByRenderObject>::make(_render_object->build(args), _layer_context->build(args));
}

}
