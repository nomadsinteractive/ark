#include "graphics/impl/renderer/render_group.h"

#include <ranges>

#include "core/base/bean_factory.h"
#include "core/components/discarded.h"

#include "graphics/components/layer.h"
#include "graphics/base/render_layer.h"

#include "renderer/inf/draw_decorator.h"

namespace ark {

namespace {

class DrawDecoratorGroupWrapper final : public DrawDecorator {
public:
    DrawDecoratorGroupWrapper(const sp<DrawDecorator>& preDraw, const sp<DrawDecorator>& postDraw)
        : _draw_decorator_pre_draw(preDraw), _draw_decorator_post_draw(postDraw) {
    }

    void preDraw(GraphicsContext& graphicsContext, const DrawingContext& context) override
    {
        if(_draw_decorator_pre_draw)
            _draw_decorator_pre_draw->preDraw(graphicsContext, context);
    }

    void postDraw(GraphicsContext& graphicsContext, const DrawingContext& context) override
    {
        if(_draw_decorator_post_draw)
            _draw_decorator_post_draw->postDraw(graphicsContext, context);
    }

    sp<DrawDecorator> _draw_decorator_pre_draw;
    sp<DrawDecorator> _draw_decorator_post_draw;
};

}

void RenderGroup::render(RenderRequest& renderRequest, const V3& position, const sp<DrawDecorator>& drawDecorator)
{
    if(drawDecorator)
    {
        sp<Renderer> firstRenderer, lastRenderer;
        sp<DrawDecoratorGroupWrapper> firstDrawDecorator;
        for(auto& [k, v] : _phrases)
            for(const sp<Renderer>& i : v.update(renderRequest.tick()))
            {
                if(!firstRenderer)
                {
                    firstDrawDecorator = sp<DrawDecoratorGroupWrapper>::make(drawDecorator, drawDecorator);
                    i->render(renderRequest, position, firstDrawDecorator);
                    firstRenderer = i;
                }
                else if(lastRenderer)
                    lastRenderer->render(renderRequest, position, nullptr);
                lastRenderer = i;
            }
        if(firstRenderer && firstRenderer != lastRenderer)
        {
            firstDrawDecorator->_draw_decorator_post_draw = nullptr;
            lastRenderer->render(renderRequest, position, sp<DrawDecorator>::make<DrawDecoratorGroupWrapper>(nullptr, drawDecorator));
        }
    }
    else
        for(auto& v : std::views::values(_phrases))
            for(const sp<Renderer>& i : v.update(renderRequest.tick()))
                i->render(renderRequest, position, nullptr);
}

void RenderGroup::addRenderer(sp<Renderer> renderer, const Traits& traits)
{
    ASSERT(renderer);
    const sp<Discarded>& discarded = traits.get<Discarded>();
    const sp<Visibility>& visibility = traits.get<Visibility>();
    const RendererType::Priority phrase = traits.getEnum<RendererType::Priority>(RendererType::PRIORITY_DEFAULT);
    _phrases[phrase].emplace_back(std::move(renderer), discarded, visibility);
}

void RenderGroup::add(const RendererType::Priority priority, sp<Renderer> renderer, sp<Boolean> discarded, sp<Boolean> visible)
{
    _phrases[priority].emplace_back(std::move(renderer), std::move(discarded), std::move(visible));
}

RenderGroup::BUILDER::BUILDER(BeanFactory& beanFactory, const document& manifest)
    : _renderers(beanFactory.makeBuilderListObject<Phrase<Renderer, RendererType::PRIORITY_DEFAULT>>(manifest, constants::RENDERER)), _render_layers(beanFactory.makeBuilderListObject<Phrase<RenderLayer, RendererType::PRIORITY_RENDER_LAYER>>(manifest, constants::RENDER_LAYER))
{
}

sp<RenderGroup> RenderGroup::BUILDER::build(const Scope& args)
{
    sp<RenderGroup> renderGroup = sp<RenderGroup>::make();
    for(const auto& i : _renderers)
        renderGroup->add(i._priority, i._renderer->build(args));
    for(const auto& i : _render_layers)
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
