#include "app/base/activity.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/util/log.h"

#include "graphics/components/layer.h"
#include "graphics/base/render_layer.h"

#include "app/base/application_context.h"
#include "app/impl/event_listener/event_listener_list.h"

namespace ark {

Activity::Activity()
    : Activity(sp<RenderGroup>::make())
{
}

Activity::Activity(sp<RenderGroup> renderGroup)
    : _render_group(renderGroup ? std::move(renderGroup) : sp<RenderGroup>::make()), _event_listeners(new EventListenerList())
{
}

Activity::~Activity()
{
    LOGD("");
}

void Activity::addRenderer(sp<Renderer> renderer, sp<Boolean> discarded, const RendererType::Priority priority)
{
    _render_group->addRenderer(std::move(renderer), std::move(discarded), priority);
}

void Activity::render(RenderRequest& renderRequest, const V3& position, const sp<DrawDecorator>& drawDecorator)
{
    _render_group->render(renderRequest, position, drawDecorator);
}

bool Activity::onEvent(const Event& event)
{
    return _event_listeners->onEvent(event);
}

void Activity::addEventListener(sp<EventListener> eventListener, sp<Boolean> discarded)
{
    _event_listeners->addEventListener(std::move(eventListener), std::move(discarded));
}

void Activity::addRenderLayer(sp<Renderer> renderLayer, sp<Boolean> discarded)
{
    if(!discarded)
        discarded = sp<Boolean>::make<BooleanByWeakRef<Renderer>>(renderLayer, 1);
    _render_group->addRenderer(std::move(renderLayer), std::move(discarded), RendererType::PRIORITY_RENDER_LAYER);
}

Activity::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _render_group(factory.ensureBuilder<RenderGroup>(manifest))
{
}

sp<Activity> Activity::BUILDER::build(const Scope& args)
{
    return sp<Activity>::make(_render_group->build(args));
}

}
