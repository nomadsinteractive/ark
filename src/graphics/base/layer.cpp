#include "graphics/base/layer.h"

#include "core/base/bean_factory.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/render_command_pipeline.h"
#include "graphics/base/render_layer.h"
#include "graphics/components/render_object.h"

#include "renderer/impl/model_loader/model_loader_cached.h"

namespace ark {

Layer::Layer()
    : Layer(nullptr)
{
}

Layer::Layer(sp<LayerContext> layerContext)
    : _layer_context(layerContext ? std::move(layerContext) : sp<LayerContext>::make())
{
}

void Layer::dispose()
{
    _layer_context = nullptr;
}

const sp<Shader>& Layer::shader() const
{
    return _layer_context->shader();
}

const SafeVar<Vec3>& Layer::position() const
{
    return _layer_context->position();
}

void Layer::setPosition(sp<Vec3> position)
{
    _layer_context->setPosition(std::move(position));
}

const SafeVar<Boolean>& Layer::visible() const
{
    return _layer_context->visible();
}

void Layer::setVisible(sp<Boolean> visible)
{
    _layer_context->visible().reset(std::move(visible));
}

const SafeVar<Boolean>& Layer::discarded() const
{
    return _layer_context->discarded();
}

sp<ModelLoader> Layer::modelLoader() const
{
    return _layer_context ? _layer_context->modelLoader() : nullptr;
}

const sp<LayerContext>& Layer::context() const
{
    return _layer_context;
}

void Layer::add(sp<Renderable> renderable, sp<Updatable> updatable, sp<Boolean> discarded)
{
    _layer_context->add(std::move(renderable), std::move(updatable), std::move(discarded));
}

void Layer::addRenderObject(const sp<RenderObject>& renderObject, const sp<Boolean>& discarded)
{
    _layer_context->add(renderObject, nullptr, discarded ? discarded : renderObject->discarded());
}

void Layer::clear()
{
    _layer_context->clear();
}

Layer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _render_layer(factory.getBuilder<RenderLayer>(manifest, constants::RENDER_LAYER)), _model_loader(factory.getBuilder<ModelLoader>(manifest, constants::MODEL_LOADER)),
      _visible(factory.getBuilder<Boolean>(manifest, constants::VISIBLE)), _position(factory.getBuilder<Vec3>(manifest, constants::POSITION)) {
}

sp<Layer> Layer::BUILDER::build(const Scope& args)
{
    sp<Vec3> position = _position.build(args);
    sp<ModelLoader> modelLoader = _model_loader.build(args);
    const sp<RenderLayer> renderLayer = _render_layer.build(args);
    //TODO: Every Layer should have been associated with a RenderLayer
    return sp<Layer>::make(renderLayer ? renderLayer->addLayerContext(std::move(modelLoader), std::move(position)) : sp<LayerContext>::make(nullptr, ModelLoaderCached::ensureCached(std::move(modelLoader)), std::move(position), _visible.build(args)));
}

}
