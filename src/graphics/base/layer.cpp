#include "graphics/base/layer.h"

#include "core/base/bean_factory.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/render_command_pipeline.h"
#include "graphics/base/render_layer.h"
#include "graphics/base/render_object.h"
#include "graphics/base/layer_context.h"

#include "renderer/impl/model_loader/model_loader_cached.h"

namespace ark {

Layer::Layer(sp<LayerContext> layerContext)
    : _layer_context(layerContext ? std::move(layerContext) : sp<LayerContext>::make())
{
}

void Layer::dispose()
{
    _layer_context = nullptr;
}

const SafeVar<Boolean>& Layer::visible() const
{
    return _layer_context->visible();
}

void Layer::setVisible(sp<Boolean> visible)
{
    _layer_context->visible().reset(std::move(visible));
}

sp<ModelLoader> Layer::modelLoader() const
{
    return _layer_context ? _layer_context->modelLoader() : nullptr;
}

const sp<LayerContext>& Layer::context() const
{
    return _layer_context;
}

void Layer::addRenderObject(const sp<RenderObject>& renderObject, const sp<Boolean>& disposed)
{
    _layer_context->add(renderObject, nullptr, disposed ? disposed : renderObject->discarded());
}

void Layer::clear()
{
    _layer_context->clear();
}

Layer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _type(Documents::getAttribute(manifest, constants::TYPE, Layer::TYPE_DYNAMIC)), _render_layer(factory.getBuilder<RenderLayer>(manifest, constants::RENDER_LAYER)),
      _model_loader(factory.getBuilder<ModelLoader>(manifest, constants::MODEL_LOADER)), _visible(factory.getBuilder<Boolean>(manifest, constants::VISIBLE)),
      _position(factory.getBuilder<Vec3>(manifest, constants::POSITION)), _render_objects(factory.makeBuilderList<RenderObject>(manifest, constants::RENDER_OBJECT))
{
}

sp<Layer> Layer::BUILDER::build(const Scope& args)
{
    sp<Vec3> position = _position->build(args);
    sp<ModelLoader> modelLoader = _model_loader->build(args);
    const sp<RenderLayer> renderLayer = _render_layer->build(args);
    //TODO: Every Layer should have been associated with a RenderLayer
    const sp<Layer> layer = sp<Layer>::make(renderLayer ? renderLayer->addLayerContext(std::move(modelLoader), std::move(position)) : sp<LayerContext>::make(nullptr, ModelLoaderCached::ensureCached(std::move(modelLoader)), std::move(position), _visible->build(args)));
    LayerContext& layerContext = layer->context();
    for(const sp<Builder<RenderObject>>& i : _render_objects)
        layerContext.add(i->build(args));
    return layer;
}

template<> ARK_API Layer::Type StringConvert::eval<Layer::Type>(const String& str)
{
    if(str == "dynamic")
        return Layer::TYPE_DYNAMIC;
    if(str == "static")
        return Layer::TYPE_STATIC;
    DCHECK(str == "transient", "Unkown layer type: %s, known types are ['static', 'dynamic', 'transient']", str.c_str());
    return Layer::TYPE_TRANSIENT;
}

}
