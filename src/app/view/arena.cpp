#include "app/view/arena.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/util/bean_utils.h"
#include "core/util/log.h"

#include "graphics/base/layer.h"
#include "graphics/base/render_layer.h"

#include "app/base/application_context.h"
#include "app/base/entity.h"
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

void Arena::addRenderer(sp<Renderer> renderer, const Traits& traits)
{
    _renderer_phrase.addRenderer(std::move(renderer), traits);
}

void Arena::render(RenderRequest& renderRequest, const V3& position)
{
    ASSERT(_view);
    _renderer_phrase.render(renderRequest, position);
}

bool Arena::onEvent(const Event& event)
{
    return _event_listeners->onEvent(event);
}

sp<Entity> Arena::makeEntity(Traits components) const
{
    components.put(_resource_loader);
    return sp<Entity>::make(std::move(components));
}

sp<Renderer> Arena::loadRenderer(const String& name, const Scope& args)
{
    sp<Renderer> renderer = load<Renderer>(name, args);
    addRenderer(renderer, Traits());
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

void Arena::addLayer(sp<Renderer> layer, sp<Boolean> discarded)
{
    _renderer_phrase.add(RendererType::PHRASE_TEXT, std::move(layer), discarded ? std::move(discarded) : layer.tryCast<Expendable>().cast<Boolean>());
}

void Arena::addRenderLayer(sp<Renderer> renderLayer, sp<Boolean> discarded)
{
    _renderer_phrase.add(RendererType::PHRASE_RENDER_LAYER, std::move(renderLayer), discarded ? std::move(discarded) : renderLayer.tryCast<Expendable>().cast<Boolean>());
}

void Arena::setView(sp<View> view)
{
    _view = std::move(view);
}

const sp<View>& Arena::view() const
{
    return _view;
}

void Arena::addView(sp<View> view, sp<Boolean> discarded)
{
    _view->addView(std::move(view), std::move(discarded));
}

Arena::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _factory(factory), _manifest(manifest), _resource_loader(factory.getBuilder<ResourceLoader>(manifest, "resource-loader")),
      _root_view(factory.ensureBuilder<View>(manifest, "root-view"))
{
}

sp<Arena> Arena::BUILDER::build(const Scope& args)
{
    sp<ResourceLoader> r1 = _resource_loader->build(args);
    sp<ResourceLoader> resourceLoader = r1 ? std::move(r1) : sp<ResourceLoader>::make(_factory);
    BeanFactory& factory = resourceLoader->beanFactory();
    sp<Arena> arena = sp<Arena>::make(_root_view->build(args), std::move(resourceLoader));

    for(const document& i : _manifest->children())
    {
        const String& name = i->name();
        if(name == constants::EVENT_LISTENER)
            arena->addEventListener(factory.ensure<EventListener>(i, args));
        else if(name == constants::RENDER_LAYER)
            arena->addRenderLayer(factory.ensure<RenderLayer>(i, args));
        else if(name == constants::VIEW)
            arena->addView(factory.ensure<View>(i, args));
        else if(name != "root-view")
        {
            CHECK_WARN(name == constants::RENDERER, "['Renderer', 'RenderLayer'] expected, \"%s\" found", name.c_str());
            arena->addRenderer(factory.ensure<Renderer>(i, args), Traits());
        }
    }
    return arena;
}

}
