#include "graphics/impl/renderer/render_group.h"

#include "core/base/bean_factory.h"
#include "graphics/components/layer.h"
#include "graphics/base/render_layer.h"

namespace ark {

struct RenderGroup::BUILDER::Phrase {

    Phrase(BeanFactory& beanFactory, const document& manifest)
        : _renderer(beanFactory.ensureBuilder<Renderer>(manifest)), _priority(Documents::getAttribute<RendererType::Priority>(manifest, "priority", RendererType::PRIORITY_DEFAULT))
    {
    }

    sp<Builder<Renderer>> _renderer;
    RendererType::Priority _priority;
};

void RenderGroup::render(RenderRequest& renderRequest, const V3& position)
{
    for(auto& [k, v] : _phrases)
        for(const sp<Renderer>& i : v.update(renderRequest.timestamp()))
            i->render(renderRequest, position);
}

void RenderGroup::addRenderer(sp<Renderer> renderer, const Traits& traits)
{
    ASSERT(renderer);
    const sp<Discarded>& droplet = traits.get<Discarded>();
    const sp<Visibility>& visibility = traits.get<Visibility>();
    const RendererType::Priority phrase = traits.getEnum<RendererType::Priority>(RendererType::PRIORITY_DEFAULT);
    _phrases[phrase].emplace_back(std::move(renderer), droplet, visibility);
}

void RenderGroup::add(RendererType::Priority priority, sp<Renderer> renderer, sp<Boolean> discarded, sp<Boolean> visible)
{
    _phrases[priority].emplace_back(std::move(renderer), std::move(discarded), std::move(visible));
}

RenderGroup::BUILDER::BUILDER(BeanFactory& beanFactory, const document& manifest)
    : _phrases(beanFactory.makeBuilderListObject<Phrase>(manifest, constants::RENDERER))
{
}

sp<RenderGroup> RenderGroup::BUILDER::build(const Scope& args)
{
    sp<RenderGroup> renderGroup = sp<RenderGroup>::make();
    for(const Phrase& i : _phrases)
        renderGroup->add(i._priority, i._renderer->build(args));
    return renderGroup;
}

RenderGroup::BUILDER_RENDERER::BUILDER_RENDERER(BeanFactory& beanFactory, const document& manifest)
    : _impl(beanFactory, manifest)
{
}

sp<Renderer> RenderGroup::BUILDER_RENDERER::build(const Scope& args)
{
    return _impl.build(args);
}

}
