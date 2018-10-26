#include "graphics/impl/renderer/renderer_by_render_object.h"

#include "core/base/bean_factory.h"
#include "core/types/safe_ptr.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/render_object.h"
#include "graphics/base/layer.h"

namespace ark {

RendererByRenderObject::RendererByRenderObject(const sp<RenderObject>& renderObject, const sp<Layer>& layer)
    : _render_object(renderObject), _layer(layer)
{
    DASSERT(_render_object);
    DASSERT(_layer);
    if(!_render_object->size())
        _layer->measure(_render_object->type()->val(), _render_object->size().ensure());
}

void RendererByRenderObject::render(RenderRequest& /*renderRequest*/, float x, float y)
{
    _layer->draw(x, y, _render_object);
}

const SafePtr<Size>& RendererByRenderObject::size()
{
    return _render_object->size();
}

RendererByRenderObject::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _render_object(factory.ensureBuilder<RenderObject>(manifest)), _layer(factory.ensureBuilder<Layer>(manifest, Constants::Attributes::LAYER))
{
}

sp<Renderer> RendererByRenderObject::BUILDER::build(const sp<Scope>& args)
{
    return sp<RendererByRenderObject>::make(_render_object->build(args), _layer->build(args));
}

}
