#include "graphics/impl/renderer/renderer_by_render_object.h"

#include "core/base/bean_factory.h"

#include "graphics/base/layer_context.h"
#include "graphics/inf/layer.h"
#include "graphics/base/render_object.h"

namespace ark {

RendererByRenderObject::RendererByRenderObject(const sp<RenderObject>& renderObject, const sp<Layer>& layer)
    : _render_object(renderObject), _layer(layer)
{
    NOT_NULL(_render_object);
    NOT_NULL(_layer);
}

void RendererByRenderObject::render(RenderRequest& /*renderRequest*/, float x, float y)
{
    _layer->layerContext()->draw(x, y, _render_object);
}

const sp<Size>& RendererByRenderObject::size()
{
    return _render_object->size();
}

RendererByRenderObject::BUILDER::BUILDER(BeanFactory& parent, const document& manifest)
    : _render_object(parent.ensureBuilder<RenderObject>(manifest)), _layer(parent.ensureBuilder<Layer>(manifest, Constants::Attributes::LAYER))
{
}

sp<Renderer> RendererByRenderObject::BUILDER::build(const sp<Scope>& args)
{
    return sp<RendererByRenderObject>::make(_render_object->build(args), _layer->build(args));
}

}
