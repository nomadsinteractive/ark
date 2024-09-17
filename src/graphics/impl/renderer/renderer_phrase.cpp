#include "graphics/impl/renderer/renderer_phrase.h"

#include "graphics/base/layer.h"
#include "graphics/base/render_layer.h"

namespace ark {

void RendererPhrase::render(RenderRequest& renderRequest, const V3& position)
{
    for(DVList<sp<Renderer>>& i : _phrases)
        for(const sp<Renderer>& j : i.update(renderRequest.timestamp()))
            j->render(renderRequest, position);
}

void RendererPhrase::addRenderer(sp<Renderer> renderer, const Traits& traits)
{
    ASSERT(renderer);
    const sp<Expendable>& droplet = traits.get<Expendable>();
    const sp<Visibility>& visibility = traits.get<Visibility>();
    const RendererType::Phrase phrase = traits.getEnum<RendererType::Phrase>(RendererType::PHRASE_DEFAULT);
    _phrases[phrase].emplace_back(std::move(renderer), droplet, visibility);
}

void RendererPhrase::add(RendererType::Phrase phrase, sp<Renderer> renderer, sp<Boolean> discarded, sp<Boolean> visible)
{
    _phrases[phrase].emplace_back(std::move(renderer), std::move(discarded), std::move(visible));
}

}
