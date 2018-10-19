#include "graphics/base/render_layer.h"

#include "core/base/bean_factory.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/render_command_pipeline.h"
#include "graphics/base/render_context.h"
#include "graphics/inf/layer.h"

namespace ark {

RenderLayer::RenderLayer(const sp<Layer>& layer)
    : _layer(layer), _render_context(layer->makeRenderContext())
{
}

void RenderLayer::render(RenderRequest& /*renderRequest*/, float x, float y)
{
    _render_context->renderRequest(V2(x, y));
}

void RenderLayer::addRenderObject(const sp<RenderObject>& renderObject, const sp<Boolean>& expired)
{
    DASSERT(renderObject);
    _render_context->addRenderObject(renderObject, expired);
}

void RenderLayer::removeRenderObject(const sp<RenderObject>& renderObject)
{
    _render_context->removeRenderObject(renderObject);
}

void RenderLayer::clear()
{
    _render_context->clear();
}

RenderLayer::BUILDER_IMPL1::BUILDER_IMPL1(BeanFactory& factory, const document& doc)
    : _layer(factory.ensureBuilder<Layer>(doc, Constants::Attributes::LAYER))
{
    for(const document& i : doc->children(Constants::Attributes::RENDER_OBJECT))
        _render_objects.push_back(factory.ensureBuilder<RenderObject>(i));
}

sp<RenderLayer> RenderLayer::BUILDER_IMPL1::build(const sp<Scope>& args)
{
    const sp<RenderLayer> renderLayer = sp<RenderLayer>::make(_layer->build(args));
    for(const sp<Builder<RenderObject>>& i : _render_objects)
    {
        const sp<RenderObject> renderObject = i->build(args);
        renderLayer->addRenderObject(renderObject, renderObject.as<Lifecycle>());
    }
    return renderLayer;
}

RenderLayer::BUILDER_IMPL2::BUILDER_IMPL2(BeanFactory& parent, const document& doc)
    : _builder_impl(parent, doc)
{
}

sp<Renderer> RenderLayer::BUILDER_IMPL2::build(const sp<Scope>& args)
{
    return _builder_impl.build(args);
}

}
