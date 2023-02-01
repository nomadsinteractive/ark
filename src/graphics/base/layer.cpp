#include "graphics/base/layer.h"

#include "core/base/bean_factory.h"
#include "core/util/holder_util.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/render_command_pipeline.h"
#include "graphics/base/render_layer.h"
#include "graphics/base/render_object.h"
#include "graphics/base/layer_context.h"
#include "graphics/impl/render_batch/render_batch_impl.h"
#include "graphics/impl/render_batch/render_batch_with_translation.h"

#include "renderer/impl/model_loader/model_loader_cached.h"

namespace ark {

Layer::Layer(sp<LayerContext> layerContext)
    : _layer_context(layerContext ? std::move(layerContext) : sp<LayerContext>::make())
{
}

void Layer::render(RenderRequest& /*renderRequest*/, const V3& position)
{
    if(_layer_context)
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

const sp<Visibility>& Layer::visible() const
{
    return _layer_context->visible();
}

void Layer::setVisible(sp<Boolean> visible)
{
    _layer_context->visible().reset(std::move(visible));
}

const sp<ModelLoader>& Layer::modelLoader() const
{
    return _layer_context ? _layer_context->modelLoader() : sp<ModelLoader>::null();
}

const sp<LayerContext>& Layer::context() const
{
    return _layer_context;
}

void Layer::addRenderObject(const sp<RenderObject>& renderObject, const sp<Boolean>& disposed)
{
    _layer_context->add(renderObject, nullptr, disposed);
}

void Layer::clear()
{
    _layer_context->clear();
}

Layer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _type(Documents::getAttribute(manifest, Constants::Attributes::TYPE, Layer::TYPE_DYNAMIC)), _render_layer(factory.getBuilder<RenderLayer>(manifest, Constants::Attributes::RENDER_LAYER)),
      _model_loader(factory.getBuilder<ModelLoader>(manifest, Constants::Attributes::MODEL)), _visible(factory.getBuilder<Boolean>(manifest, Constants::Attributes::VISIBLE)),
      _position(factory.getBuilder<Vec3>(manifest, Constants::Attributes::POSITION)), _render_objects(factory.makeBuilderList<RenderObject>(manifest, Constants::Attributes::RENDER_OBJECT))
{
}

sp<Layer> Layer::BUILDER::build(const Scope& args)
{
    sp<Vec3> position = _position->build(args);
    sp<ModelLoader> modelLoader = _model_loader->build(args);
    sp<RenderBatch> renderBatch = position ? sp<RenderBatch>::make<RenderBatchWithTranslation>(sp<RenderBatchImpl>::make(), std::move(position)) : sp<RenderBatch>::make<RenderBatchImpl>();
    const sp<RenderLayer> renderLayer = _render_layer->build(args);
    const sp<Layer> layer = sp<Layer>::make(renderLayer ? renderLayer->makeLayerContext(std::move(renderBatch), std::move(modelLoader)) : sp<LayerContext>::make(std::move(renderBatch), ModelLoaderCached::ensureCached(std::move(modelLoader)), _visible->build(args)));
    LayerContext& layerContext = layer->context();
    for(const sp<Builder<RenderObject>>& i : _render_objects)
        layerContext.add(i->build(args));
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

template<> ARK_API Layer::Type StringConvert::to<String, Layer::Type>(const String& str)
{
    if(str == "dynamic")
        return Layer::TYPE_DYNAMIC;
    if(str == "static")
        return Layer::TYPE_STATIC;
    DCHECK(str == "transient", "Unkown layer type: %s, known types are ['static', 'dynamic', 'transient']", str.c_str());
    return Layer::TYPE_TRANSIENT;
}

}
