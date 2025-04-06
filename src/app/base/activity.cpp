#include "app/base/activity.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/util/log.h"

#include "graphics/components/layer.h"
#include "graphics/base/render_layer.h"

#include "app/base/application_context.h"
#include "app/base/arena.h"
#include "app/base/entity.h"
#include "app/impl/event_listener/event_listener_list.h"
#include "app/view/view.h"


namespace ark {

Activity::Activity(sp<View> view, sp<RenderGroup> renderGroup, sp<ResourceLoader> resourceLoader)
    : _view(std::move(view)), _render_group(std::move(renderGroup)), _resource_loader(std::move(resourceLoader)), _event_listeners(new EventListenerList())
{
    _view->markAsTopView();
}

Activity::~Activity()
{
    LOGD("");
}

void Activity::addRenderer(sp<Renderer> renderer, const Traits& traits)
{
    _render_group->addRenderer(std::move(renderer), traits);
}

void Activity::render(RenderRequest& renderRequest, const V3& position, const sp<DrawDecorator>& drawDecorator)
{
    ASSERT(_view);
    _render_group->render(renderRequest, position, drawDecorator);
}

bool Activity::onEvent(const Event& event)
{
    return _event_listeners->onEvent(event);
}

sp<Entity> Activity::makeEntity(Traits components) const
{
    components.put(_resource_loader);
    return sp<Entity>::make(std::move(components));
}

sp<Arena> Activity::makeArena() const
{
    return sp<Arena>::make(_render_group, _resource_loader);
}

Box Activity::getReference(const String& id) const
{
    return _resource_loader->refs()->get(id);
}

const sp<ResourceLoader>& Activity::resourceLoader() const
{
    CHECK(_resource_loader, "Trying to get ResourceLoader on a discarded Activity");
    return _resource_loader;
}

sp<BoxBundle> Activity::refs() const
{
    CHECK(_resource_loader, "Trying to get ResourceLoader on a discarded Activity");
    return _resource_loader->refs();
}

sp<BoxBundle> Activity::packages() const
{
    CHECK(_resource_loader, "Trying to get ResourceLoader on a discarded Activity");
    return _resource_loader->packages();
}

void Activity::addEventListener(sp<EventListener> eventListener, sp<Boolean> discarded)
{
    _event_listeners->addEventListener(std::move(eventListener), std::move(discarded));
}

void Activity::pushEventListener(sp<EventListener> eventListener, sp<Boolean> discarded)
{
    _event_listeners->pushEventListener(std::move(eventListener), std::move(discarded));
}

void Activity::addRenderLayer(sp<Renderer> renderLayer, sp<Boolean> discarded)
{
    if(!discarded)
        discarded = sp<Boolean>::make<BooleanByWeakRef<Renderer>>(renderLayer, 1);
    _render_group->add(RendererType::PRIORITY_RENDER_LAYER, std::move(renderLayer), std::move(discarded));
}

void Activity::setView(sp<View> view)
{
    _view = std::move(view);
}

const sp<View>& Activity::view() const
{
    return _view;
}

void Activity::addView(sp<View> view, sp<Boolean> discarded)
{
    _view->addView(std::move(view), std::move(discarded));
}

void Activity::addEntity(sp<Entity> entity)
{
    _entities.push_back(std::move(entity));
}

Activity::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _factory(factory), _manifest(manifest), _resource_loader(factory.getBuilder<ResourceLoader>(manifest, "resource-loader")),
      _root_view(factory.ensureBuilder<View>(manifest, "root-view")), _render_group(factory.ensureBuilder<RenderGroup>(manifest)), _entities(factory.makeBuilderList<Entity>(manifest, constants::ENTITY))
{
}

sp<Activity> Activity::BUILDER::build(const Scope& args)
{
    sp<ResourceLoader> r1 = _resource_loader.build(args);
    sp<ResourceLoader> resourceLoader = r1 ? std::move(r1) : sp<ResourceLoader>::make(_factory);
    BeanFactory& factory = resourceLoader->beanFactory();
    sp<Activity> activity = sp<Activity>::make(_root_view->build(args), _render_group->build(args), std::move(resourceLoader));

    for(const document& i : _manifest->children())
    {
        if(const String& name = i->name(); name == constants::EVENT_LISTENER)
            activity->addEventListener(factory.ensure<EventListener>(i, args));
        else if(name == constants::VIEW)
            activity->addView(factory.ensure<View>(i, args));
    }

    for(const builder<Entity>& i : _entities)
        activity->addEntity(i->build(args));

    return activity;
}

}
