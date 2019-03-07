#include "graphics/base/render_layer.h"

#include "core/base/bean_factory.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/render_command_pipeline.h"
#include "graphics/base/layer_context.h"
#include "graphics/base/layer.h"

namespace ark {

RenderLayer::RenderLayer(const sp<Layer>& layer)
    : _layer(layer), _layer_context(layer->makeContext())
{
}

void RenderLayer::render(RenderRequest& /*renderRequest*/, float x, float y)
{
    _layer_context->renderRequest(V2(x, y));
}

void RenderLayer::draw(float x, float y, const sp<RenderObject>& renderObject)
{
    _layer_context->draw(x, y, renderObject);
}

void RenderLayer::addRenderObject(const sp<RenderObject>& renderObject, const sp<Disposable>& lifecycle)
{
    DASSERT(renderObject);
    _layer_context->addRenderObject(renderObject, lifecycle);
}

void RenderLayer::removeRenderObject(const sp<RenderObject>& renderObject)
{
    _layer_context->removeRenderObject(renderObject);
}

const sp<Layer>& RenderLayer::layer() const
{
    return _layer;
}

void RenderLayer::clear()
{
    _layer_context->clear();
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
        renderLayer->addRenderObject(renderObject, renderObject.as<Disposable>());
    }
    return renderLayer;
}

RenderLayer::BUILDER_IMPL2::BUILDER_IMPL2(BeanFactory& factory, const document& manifest)
    : _builder_impl(factory, manifest)
{
}

sp<Renderer> RenderLayer::BUILDER_IMPL2::build(const sp<Scope>& args)
{
    return _builder_impl.build(args);
}

}
