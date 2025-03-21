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
};

struct Arena::RenderLayerBundle final : BoxBundle {

    Box get(const String& name) override
    {
        const auto iter = _stub->_render_layers.find(name);
        if(iter == _stub->_render_layers.end())
        {
            sp<RenderLayer> renderLayer = _stub->_resource_loader->load<RenderLayer>(name, {});
            _stub->_render_layers.insert(std::make_pair(name, renderLayer));
            return Box(renderLayer);
        }
        return Box(iter->second);
    }

    sp<Stub> _stub;
};

Arena::Arena(sp<RenderGroup> renderGroup, sp<ResourceLoader> resourceLoader)
    : _stub(sp<Stub>::make(Stub{std::move(renderGroup), std::move(resourceLoader)}))
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
