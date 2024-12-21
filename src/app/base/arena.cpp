#include "app/base/arena.h"

#include "graphics/base/render_layer.h"
#include "graphics/impl/renderer/render_group.h"

namespace ark {

struct Arena::Stub {
    sp<RendererPhrase> _render_phrases;
    sp<ResourceLoader> _resource_loader;
    sp<Scope> _scope;

    std::map<String, sp<Layer>> _layers;
    std::map<String, sp<RenderLayer>> _render_layers;
};

Arena::Arena(sp<RendererPhrase> renderPhrases, sp<ResourceLoader> resourceLoader, sp<Scope> args)
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
