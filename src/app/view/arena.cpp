#include "app/view/arena.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/util/bean_utils.h"
#include "core/util/holder_util.h"
#include "core/util/log.h"

#include "graphics/base/layer.h"
#include "graphics/base/render_layer.h"

#include "app/base/application_context.h"
#include "app/impl/event_listener/event_listener_list.h"
#include "app/view/view.h"


namespace ark {

Arena::Arena(sp<View> view, sp<ResourceLoader> resourceLoader)
    : _view(std::move(view)), _resource_loader(std::move(resourceLoader)), _event_listeners(new EventListenerList())
{
    _view->markAsTopView();
}

Arena::~Arena()
{
    LOGD("");
}

void Arena::addRenderer(const sp<Renderer>& renderer)
{
    _renderers.push_back(renderer);
}

void Arena::render(RenderRequest& renderRequest, const V3& position)
{
    ASSERT(_view);
    _view->render(renderRequest, position);
    for(const sp<Renderer>& i : _renderers.update(renderRequest.timestamp()))
        i->render(renderRequest, position);
    for(const sp<Renderer>& i : _layers.update(renderRequest.timestamp()))
        i->render(renderRequest, position);
    for(const sp<Renderer>& i : _render_layers.update(renderRequest.timestamp()))
        i->render(renderRequest, position);
}

bool Arena::onEvent(const Event& event)
{
    return _event_listeners->onEvent(event);
}

void Arena::traverse(const Holder::Visitor& visitor)
{
    HolderUtil::visit(_view, visitor);
    for(const auto& i : _layers.items())
        HolderUtil::visit(i._item, visitor);
    for(const auto& i : _render_layers.items())
        HolderUtil::visit(i._item, visitor);
}

sp<Renderer> Arena::loadRenderer(const String& name, const Scope& args)
{
    const sp<Renderer> renderer = load<Renderer>(name, args);
    addRenderer(renderer);
    return renderer;
}

Box Arena::getReference(const String& id) const
{
    return _resource_loader->refs()->get(id);
}

const sp<ResourceLoader>& Arena::resourceLoader() const
{
    DCHECK(_resource_loader, "Trying to get ResourceLoader on a disposed Arena");
    return _resource_loader;
}

sp<BoxBundle> Arena::refs() const
{
    DCHECK(_resource_loader, "Trying to get ResourceLoader on a disposed Arena");
    return _resource_loader->refs();
}

sp<BoxBundle> Arena::layers() const
{
    DCHECK(_resource_loader, "Trying to get ResourceLoader on a disposed Arena");
    return _resource_loader->layers();
}

sp<BoxBundle> Arena::renderLayers() const
{
    DCHECK(_resource_loader, "Trying to get ResourceLoader on a disposed Arena");
    return _resource_loader->renderLayers();
}

sp<BoxBundle> Arena::packages() const
{
    DCHECK(_resource_loader, "Trying to get ResourceLoader on a disposed Arena");
    return _resource_loader->packages();
}

void Arena::addEventListener(sp<EventListener> eventListener, sp<Boolean> disposed)
{
    _event_listeners->addEventListener(std::move(eventListener), std::move(disposed));
}

void Arena::pushEventListener(sp<EventListener> eventListener, sp<Boolean> disposed)
{
    _event_listeners->pushEventListener(std::move(eventListener), std::move(disposed));
}

void Arena::addLayer(sp<Renderer> layer)
{
    _layers.push_back(std::move(layer));
}

void Arena::addRenderLayer(sp<Renderer> renderLayer)
{
    _render_layers.push_back(std::move(renderLayer));
}

void Arena::setView(sp<View> view)
{
    _view = std::move(view);
}

const sp<View>& Arena::view() const
{
    return _view;
}

void Arena::addView(sp<View> view, sp<Boolean> disposable)
{
    _view->addView(std::move(view), std::move(disposable));
}

Arena::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _factory(factory), _manifest(manifest), _resource_loader(factory.getBuilder<ResourceLoader>(manifest, "resource-loader")),
      _view(factory.getBuilder<View>(manifest, Constants::Attributes::VIEW))
{
}

sp<Arena> Arena::BUILDER::build(const Scope& args)
{
    sp<ResourceLoader> r1 = _resource_loader->build(args);
    sp<ResourceLoader> resourceLoader = r1 ? std::move(r1) : sp<ResourceLoader>::make(_factory);
    BeanFactory& factory = resourceLoader->beanFactory();
    sp<Arena> arena = sp<Arena>::make(_view->build(args), std::move(resourceLoader));

    for(const document& i : _manifest->children())
    {
        const String& name = i->name();
        if(name == Constants::Attributes::EVENT_LISTENER)
            arena->addEventListener(factory.ensure<EventListener>(i, args));
        else if(name == Constants::Attributes::RENDER_LAYER)
            arena->addRenderLayer(factory.ensureDecorated<Renderer, RenderLayer>(i, args));
        else if(name == Constants::Attributes::LAYER)
            arena->addLayer(factory.ensureDecorated<Renderer, Layer>(i, args));
        else if(name != Constants::Attributes::VIEW)
        {
            WARN(name == Constants::Attributes::RENDERER, "['Renderer', 'RenderLayer', 'Layer'] expected, \"%s\" found", name.c_str());
            arena->addRenderer(factory.ensure<Renderer>(i, args));
        }
    }
    return arena;
}

}
