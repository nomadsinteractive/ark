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
    : _render_object(renderObject), _layer_context(layerContext)
{
    DASSERT(_render_object);
    DASSERT(layerContext);
    if(!_render_object->size())
        measure(_render_object->type()->val(), _render_object->size().ensure());
}

void RendererByRenderObject::render(RenderRequest& /*renderRequest*/, float x, float y)
{
    _layer_context->draw(x, y, _render_object);
}

const SafePtr<Size>& RendererByRenderObject::size()
{
    return _render_object->size();
}

void RendererByRenderObject::measure(int32_t type, Size& size)
{
    const Metrics metrics = _layer_context->renderModel()->measure(type);
    size.setWidth(metrics.size.x());
    size.setHeight(metrics.size.y());
}

RendererByRenderObject::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _render_object(factory.ensureBuilder<RenderObject>(manifest)), _layer_context(sp<LayerContext::BUILDER>::make(factory, manifest, false))
{
}

sp<Renderer> RendererByRenderObject::BUILDER::build(const sp<Scope>& args)
{
    return sp<RendererByRenderObject>::make(_render_object->build(args), _layer_context->build(args));
}

}
