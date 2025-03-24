#include "app/base/arena.h"

#include "core/base/resource_loader.h"
#include "core/inf/dictionary.h"

#include "graphics/base/render_layer.h"
#include "graphics/impl/renderer/render_group.h"

namespace ark {

struct Arena::Stub {
    sp<RenderGroup> _render_group;
    sp<ResourceLoader> _resource_loader;

    Map<String, sp<Layer>> _layers;
    Map<String, sp<RenderLayer>> _render_layers;

    sp<Layer> getLayer(const String& name)
    {
        const auto iter = _layers.find(name);
        if(iter == _layers.end())
        {
            sp<Layer> layer = _resource_loader->load<Layer>(name, {});
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

Arena::Arena(sp<RenderGroup> renderGroup, sp<ResourceLoader> resourceLoader)
    : _stub(sp<Stub>::make(Stub{std::move(renderGroup), std::move(resourceLoader)})), _layers(sp<LayerBundle>::make(_stub)), _render_layers(sp<RenderLayerBundle>::make(_stub))
{
}

const sp<BoxBundle>& Arena::renderLayers() const
{
    return _render_layers;
}

const sp<BoxBundle>& Arena::layers() const
{
    return _layers;
}

}
