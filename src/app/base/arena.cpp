#include "app/base/arena.h"

#include "core/base/resource_loader.h"
#include "core/inf/dictionary.h"

#include "graphics/base/render_layer.h"
#include "graphics/impl/renderer/render_group.h"

namespace ark {

struct Arena::Stub {
    sp<RenderGroup> _render_phrases;
    sp<ResourceLoader> _resource_loader;
    sp<Scope> _scope;

    std::map<String, sp<Layer>> _layers;
    std::map<String, sp<RenderLayer>> _render_layers;
};

struct Arena::RenderLayerBundle final : BoxBundle {

    Box get(const String& name) override
    {
        const auto iter = _stub->_render_layers.find(name);
        if(iter == _stub->_render_layers.end())
        {
            sp<RenderLayer> renderLayer = _stub->_resource_loader->load<RenderLayer>(name, *_stub->_scope);
            _stub->_render_layers.insert(std::make_pair(name, renderLayer));
            return Box(renderLayer);
        }
        return Box(iter->second);
    }

    sp<Stub> _stub;
};

Arena::Arena(sp<RenderGroup> renderPhrases, sp<ResourceLoader> resourceLoader, sp<Scope> args)
    : _stub(sp<Stub>::make(Stub{std::move(renderPhrases), std::move(resourceLoader), std::move(args)}))
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
