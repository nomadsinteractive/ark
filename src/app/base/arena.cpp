#include "app/base/arena.h"

#include "core/base/resource_loader.h"
#include "core/inf/dictionary.h"

#include "graphics/base/render_layer.h"

namespace ark {

struct Arena::Stub {
    sp<ResourceLoader> _resource_loader;

    Map<String, sp<Layer>> _layers;
    Map<String, sp<RenderLayer>> _render_layers;

    sp<Layer> getLayer(const String& name)
    {
        const auto iter = _layers.find(name);
        if(iter == _layers.end())
        {
            sp<Layer> layer = _resource_loader->load<Layer>(name, {});
            if(layer)
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
            if(renderLayer)
                _render_layers.insert(std::make_pair(name, renderLayer));
            return renderLayer;
        }
        return iter->second;
    }
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

Arena::Arena(sp<ResourceLoader> resourceLoader)
    : Niche("layer-name"), _stub(sp<Stub>::make(Stub{std::move(resourceLoader)})), _layers(sp<LayerBundle>::make(_stub)), _render_layers(sp<RenderLayerBundle>::make(_stub))
{
}

const sp<BoxBundle>& Arena::renderLayers() const
{
    return _render_layers;
}

void Arena::onPoll(Wirable::WiringContext& context, const StringView value)
{
    sp<Layer> layer = _stub->getLayer(value.data());
    CHECK(layer, "Layer(%s) not found", value.data());
    context.setComponent(std::move(layer));
}

const sp<BoxBundle>& Arena::layers() const
{
    return _layers;
}

}
