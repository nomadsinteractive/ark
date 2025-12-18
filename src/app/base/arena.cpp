#include "app/base/arena.h"

#include "core/base/resource_loader.h"
#include "core/components/discarded.h"
#include "core/inf/dictionary.h"

#include "graphics/base/render_layer.h"
#include "graphics/util/renderer_type.h"

namespace ark {

struct Arena::Stub {
    Stub(sp<ResourceLoader> resourceLoader, sp<Boolean> discarded, sp<Renderer> renderer, Map<String, sp<RenderLayer>> renderLayers, Map<String, sp<Layer>> layers)
        : _resource_loader(std::move(resourceLoader)), _renderer(std::move(renderer)), _render_layers(std::move(renderLayers)), _layers(std::move(layers)), _discarded(sp<Discarded>::make(std::move(discarded)))
    {
        CHECK(!_renderer || _renderer.isInstance<Renderer::Group>(), "Renderer of an Arena should be nullptr or instance of Renderer::Group");
    }

    sp<Layer> getLayer(const String& name)
    {
        const auto iter = _layers.find(name);
        if(iter == _layers.end())
        {
            const auto [_, renderLayerName] = name.rcut('@');
            sp<Layer> layer = getRenderLayer(renderLayerName)->makeLayer();
            _layers.insert(std::make_pair(name, layer));
            return layer;
        }
        return iter->second;
    }

    sp<RenderLayer> getRenderLayer(const String& name)
    {
        const auto iter = _render_layers.find(name);
        if(iter == _render_layers.end())
        {
            sp<RenderLayer> renderLayer = _resource_loader->load<RenderLayer>(name, {});
            CHECK(renderLayer, "Cannot get RenderLayer \"%s\"", name.c_str());
            _render_layers.insert(std::make_pair(name, renderLayer));
            if(_renderer)
                RendererType::addRenderer(_renderer, renderLayer, {_discarded, RendererType::PRIORITY_RENDER_LAYER});
            return renderLayer;
        }
        return iter->second;
    }

    sp<ResourceLoader> _resource_loader;
    sp<Renderer> _renderer;

    Map<String, sp<RenderLayer>> _render_layers;
    Map<String, sp<Layer>> _layers;

    sp<Discarded> _discarded;
};

class Arena::LayerBundle final : public BoxBundle {
public:
    LayerBundle(const sp<Stub>& stub)
        : _stub(stub) {
    }

    Box get(const String& name) override
    {
        return Box(_stub->getLayer(name));
    }

private:
    sp<Stub> _stub;
};

class Arena::RenderLayerBundle final : public BoxBundle {
public:
    RenderLayerBundle(const sp<Stub>& stub)
        : _stub(stub) {
    }

    Box get(const String& name) override
    {
        return Box(_stub->getRenderLayer(name));
    }

private:
    sp<Stub> _stub;
};

Arena::Arena(sp<ResourceLoader> resourceLoader, sp<Boolean> discarded, sp<Renderer> renderer, Map<String, sp<RenderLayer>> renderLayers, Map<String, sp<Layer>> layers)
    : _stub(sp<Stub>::make(std::move(resourceLoader), std::move(discarded), std::move(renderer), std::move(renderLayers), std::move(layers))), _layers(sp<BoxBundle>::make<LayerBundle>(_stub)), _render_layers(sp<BoxBundle>::make<RenderLayerBundle>(_stub))
{
}

Arena::~Arena()
{
    discard();
}

void Arena::discard() const
{
    _stub->_discarded->discard();
}

const sp<BoxBundle>& Arena::layers() const
{
    return _layers;
}

const sp<BoxBundle>& Arena::renderLayers() const
{
    return _render_layers;
}

void Arena::addLayer(String name, sp<Layer> layer) const
{
    _stub->_layers.insert({std::move(name), std::move(layer)});
}

void Arena::addRenderLayer(String name, sp<RenderLayer> renderLayer) const
{
    _stub->_render_layers.insert({std::move(name), std::move(renderLayer)});
}

void Arena::onPoll(Wirable::WiringContext& context, const document& component)
{
    if(const String layerName = Documents::getAttribute(component, "layer-name"))
    {
        sp<Layer> layer = _stub->getLayer(layerName);
        CHECK(layer, "Layer(%s) not found", layerName.c_str());
        context.setInterface(std::move(layer));
    }
    if(const String renderLayerName = Documents::getAttribute(component, "render-layer-name"))
    {
        sp<RenderLayer> renderLayer = _stub->getRenderLayer(renderLayerName);
        CHECK(renderLayer, "RenderLayer(%s) not found", renderLayerName.c_str());
        context.setInterface(std::move(renderLayer));
    }
}

}
