#include "graphics/impl/renderer/render_group.h"

#include "core/base/bean_factory.h"
#include "graphics/base/layer.h"
#include "graphics/base/render_layer.h"

namespace ark {

struct RendererPhrase::BUILDER::Phrase {

    Phrase(BeanFactory& beanFactory, const document& manifest, RendererType::Phrase phrase)
        : _renderer(beanFactory.ensureBuilder<Renderer>(manifest)), _phrase(phrase)
    {
    }

    sp<Builder<Renderer>> _renderer;
    RendererType::Phrase _phrase;
};

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

RendererPhrase::BUILDER::BUILDER(BeanFactory& beanFactory, const document& manifest)
    : _phrases{
        beanFactory.makeBuilderListObject<Phrase>(manifest, constants::RENDERER, RendererType::PHRASE_DEFAULT),
        beanFactory.makeBuilderListObject<Phrase>(manifest, constants::RENDER_TARGET, RendererType::PHRASE_DEFAULT),
        beanFactory.makeBuilderListObject<Phrase>(manifest, "widget", RendererType::PHRASE_WIDGET),
        beanFactory.makeBuilderListObject<Phrase>(manifest, constants::RENDER_LAYER, RendererType::PHRASE_RENDER_LAYER)
    }
{
}

sp<RendererPhrase> RendererPhrase::BUILDER::build(const Scope& args)
{
    sp<RendererPhrase> renderGroup = sp<RendererPhrase>::make();
    for(const std::vector<Phrase>& i : _phrases)
        for(const Phrase& j : i)
            renderGroup->add(j._phrase, j._renderer->build(args));
    return renderGroup;
}

RendererPhrase::BUILDER_RENDERER::BUILDER_RENDERER(BeanFactory& beanFactory, const document& manifest)
    : _impl(beanFactory, manifest)
{
}

sp<Renderer> RendererPhrase::BUILDER_RENDERER::build(const Scope& args)
{
    return _impl.build(args);
}

}
