#include "graphics/impl/renderer/renderer_by_render_object.h"

#include "core/base/bean_factory.h"
#include "core/types/safe_ptr.h"

#include "graphics/base/layer.h"
#include "graphics/base/layer_context.h"
#include "graphics/base/metrics.h"
#include "graphics/base/render_object.h"
#include "graphics/base/render_layer.h"
#include "graphics/base/size.h"

#include "renderer/base/model.h"
#include "renderer/inf/model_loader.h"

namespace ark {

RendererByRenderObject::RendererByRenderObject(sp<LayerContext> layerContext, sp<RenderObject> renderObject)
    : _layer_context(std::move(layerContext)), _render_object(std::move(renderObject))
{
    DASSERT(_layer_context);
    if(!_render_object->_size)
        measure(_render_object->_size.ensure());
}

RendererByRenderObject::~RendererByRenderObject()
{
    _render_object->dispose();
}

void RendererByRenderObject::render(RenderRequest& /*renderRequest*/, const V3& position)
{
    if(!_renderable)
    {
         _renderable = sp<RenderablePassive>::make(_render_object);
         _layer_context->add(_renderable, _render_object->disposed());
    }
    _renderable->requestUpdate(position);
}

const sp<Size>& RendererByRenderObject::size()
{
    return _render_object->_size.ensure();
}

void RendererByRenderObject::measure(Size& size)
{
    const Metrics& metrics = _layer_context->modelLoader()->loadModel(_render_object->type()->val())->metrics();
    size.setWidth(metrics.width());
    size.setHeight(metrics.height());
}

RendererByRenderObject::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _render_object(factory.ensureConcreteClassBuilder<RenderObject>(manifest, Constants::Attributes::RENDER_OBJECT)), _layer_context(sp<LayerContext::BUILDER>::make(factory, manifest, Layer::TYPE_UNSPECIFIED))
{
}

sp<Renderer> RendererByRenderObject::BUILDER::build(const Scope& args)
{
    return sp<RendererByRenderObject>::make(_layer_context->build(args), _render_object->build(args));
}

}
