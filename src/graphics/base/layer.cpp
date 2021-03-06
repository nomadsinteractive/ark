#include "graphics/base/layer.h"

#include "core/base/bean_factory.h"
#include "core/util/holder_util.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/render_command_pipeline.h"
#include "graphics/base/render_layer.h"
#include "graphics/base/layer_context.h"

namespace ark {

Layer::Layer(const sp<LayerContext>& layerContext)
    : _layer_context(layerContext)
{
}

void Layer::render(RenderRequest& /*renderRequest*/, const V3& position)
{
    _layer_context->renderRequest(position);
}

void Layer::traverse(const Holder::Visitor& visitor)
{
    if(_layer_context)
        _layer_context->traverse(visitor);
}

void Layer::dispose()
{
    _layer_context = nullptr;
}

const sp<LayerContext>& Layer::context() const
{
    return _layer_context;
}

void Layer::addRenderObject(const sp<RenderObject>& renderObject, const sp<Boolean>& disposed)
{
    _layer_context->addRenderObject(renderObject, disposed);
}

void Layer::clear()
{
    _layer_context->clear();
}

Layer::BUILDER_IMPL1::BUILDER_IMPL1(BeanFactory& factory, const document& manifest)
    : _type(Documents::getAttribute(manifest, Constants::Attributes::TYPE, Layer::TYPE_DYNAMIC)), _render_layer(factory.ensureBuilder<RenderLayer>(manifest, Constants::Attributes::RENDER_LAYER))
{
    for(const document& i : manifest->children(Constants::Attributes::RENDER_OBJECT))
        _render_objects.push_back(factory.ensureBuilder<RenderObject>(i));
}

sp<Layer> Layer::BUILDER_IMPL1::build(const Scope& args)
{
    const sp<RenderLayer> renderLayer = _render_layer->build(args);
    const sp<Layer> layer = sp<Layer>::make(renderLayer->makeContext(_type));
    const sp<LayerContext>& layerContext = layer->context();
    for(const sp<Builder<RenderObject>>& i : _render_objects)
        layerContext->addRenderObject(i->build(args));
    return layer;
}

Layer::BUILDER_IMPL2::BUILDER_IMPL2(BeanFactory& factory, const document& manifest)
    : _builder_impl(factory, manifest)
{
}

sp<Renderer> Layer::BUILDER_IMPL2::build(const Scope& args)
{
    return _builder_impl.build(args);
}

template<> ARK_API Layer::Type Conversions::to<String, Layer::Type>(const String& str)
{
    if(str == "dynamic")
        return Layer::TYPE_DYNAMIC;
    if(str == "static")
        return Layer::TYPE_STATIC;
    DCHECK(str == "transient", "Unkown layer type: %s, known types are ['static', 'dynamic', 'transient']", str.c_str());
    return Layer::TYPE_TRANSIENT;
}

}
