#include "graphics/base/layer.h"

#include "core/base/bean_factory.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/render_command_pipeline.h"
#include "graphics/base/render_layer.h"
#include "graphics/base/layer_context.h"

namespace ark {

Layer::Layer(const sp<RenderLayer>& renderer)
    : _render_layer(renderer), _layer_context(renderer->makeContext())
{
}

Layer::Layer(const sp<RenderLayer::Stub>& stub)
    : _render_layer(sp<RenderLayer>::adopt(new RenderLayer(stub))), _layer_context(stub->_layer_context)
{
}
void Layer::render(RenderRequest& /*renderRequest*/, float x, float y)
{
    _layer_context->renderRequest(V2(x, y));
}

void Layer::draw(float x, float y, const sp<RenderObject>& renderObject)
{
    _layer_context->draw(x, y, renderObject);
}

const sp<RenderLayer>& Layer::renderer() const
{
    return _render_layer;
}

const sp<LayerContext>& Layer::context() const
{
    return _layer_context;
}

void Layer::addRenderObject(const sp<RenderObject>& renderObject, const sp<Boolean>& disposed)
{
    _layer_context->addRenderObject(renderObject, disposed);
}

void Layer::removeRenderObject(const sp<RenderObject>& renderObject)
{
    _layer_context->removeRenderObject(renderObject);
}

void Layer::clear()
{
    _layer_context->clear();
}

Layer::BUILDER_IMPL1::BUILDER_IMPL1(BeanFactory& factory, const document& manifest)
    : _render_layer(factory.ensureBuilder<RenderLayer>(manifest, Constants::Attributes::RENDER_LAYER))
{
    for(const document& i : manifest->children(Constants::Attributes::RENDER_OBJECT))
        _render_objects.push_back(factory.ensureBuilder<RenderObject>(i));
}

sp<Layer> Layer::BUILDER_IMPL1::build(const sp<Scope>& args)
{
    const sp<Layer> layer = sp<Layer>::make(_render_layer->build(args));
    const sp<LayerContext>& layerContext = layer->context();
    for(const sp<Builder<RenderObject>>& i : _render_objects)
        layerContext->addRenderObject(i->build(args));
    return layer;
}

Layer::BUILDER_IMPL2::BUILDER_IMPL2(BeanFactory& factory, const document& manifest)
    : _builder_impl(factory, manifest)
{
}

sp<Renderer> Layer::BUILDER_IMPL2::build(const sp<Scope>& args)
{
    return _builder_impl.build(args);
}

}
