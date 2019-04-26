#include "app/view/arena.h"

#include "core/base/bean_factory.h"
#include "core/util/bean_utils.h"
#include "core/util/log.h"

#include "graphics/base/layer.h"
#include "graphics/base/render_layer.h"

#include "app/base/application_context.h"
#include "app/impl/event_listener/event_listener_list.h"


namespace ark {

Arena::Arena(const sp<Renderer>& rootView, const sp<ResourceLoader>& resourceLoader)
    : _event_listeners(new EventListenerList()), _view_group(rootView), _renderer(rootView), _resource_loader(resourceLoader)
{
    DCHECK(_view_group, "Arena's renderer delegate must be ViewGroup");
}

Arena::~Arena()
{
    LOGD("");
}

void Arena::addRenderer(const sp<Renderer>& renderer)
{
    DASSERT(_view_group);
    _view_group->addRenderer(renderer);
}

void Arena::render(RenderRequest& renderRequest, float x, float y)
{
    DASSERT(_renderer);
    _renderer->render(renderRequest, x, y);
    for(const sp<Renderer>& i : _layers)
        i->render(renderRequest, x, y);
}

bool Arena::onEvent(const Event& event)
{
    DASSERT(_view_group);
    return _view_group->onEvent(event, 0.0f, 0.0f) || _event_listeners->onEvent(event);
}

sp<Renderer> Arena::loadRenderer(const String& name, const sp<Scope>& args)
{
    const sp<Renderer> renderer = load<Renderer>(name, args);
    addRenderer(renderer);
    return renderer;
}

const sp<ResourceLoader>& Arena::resourceLoader() const
{
    DCHECK(_resource_loader, "Trying to get ResourceLoader on a disposed Arena");
    return _resource_loader;
}

void Arena::addEventListener(const sp<EventListener>& eventListener)
{
    _event_listeners->addEventListener(eventListener);
}

void Arena::addLayer(const sp<Renderer>& layer)
{
    _layers.push_back(layer);
}

void Arena::setView(const sp<Renderer>& view)
{
    DCHECK(view.is<ViewGroup>(), "Arena's renderer delegate must be ViewGroup");
    _renderer = view;
    _view_group = view.as<ViewGroup>();
}

const sp<ViewGroup>& Arena::view() const
{
    return _view_group;
}

void Arena::dispose()
{
    _layers.clear();
    _resource_loader = nullptr;
}

Arena::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _factory(factory), _manifest(manifest), _resource_loader(factory.getBuilder<ResourceLoader>(manifest, "resource-loader")),
      _layout(factory.getBuilder<Layout>(manifest, Constants::Attributes::LAYOUT)),
      _layout_param(factory.ensureBuilder<LayoutParam>(manifest)),
      _background(Documents::getAttribute(manifest, Constants::Attributes::BACKGROUND)),
      _view(Documents::getAttribute(manifest, Constants::Attributes::VIEW))
{
}

sp<Arena> Arena::BUILDER::build(const sp<Scope>& args)
{
    const sp<ResourceLoader> r1 = _resource_loader->build(args);
    const sp<ResourceLoader> resourceLoader = r1 ? r1 : sp<ResourceLoader>::make(_factory);
    const sp<Renderer> background = _background.empty() ? nullptr : resourceLoader->load<Renderer>(_background, args);
    const sp<Renderer> view = _view ? resourceLoader->load<Renderer>(_view, args) : nullptr;
    const sp<Arena> arena = sp<Arena>::make(view ? view : sp<ViewGroup>::make(background, _layout->build(args), _layout_param->build(args)).cast<Renderer>(), resourceLoader);

    BeanFactory& factory = resourceLoader->beanFactory();
    for(const document& i : _manifest->children())
    {
        const String& name = i->name();
        if(name == Constants::Attributes::EVENT_LISTENER)
            arena->addEventListener(factory.ensure<EventListener>(i, args));
        else if(name == Constants::Attributes::RENDER_LAYER)
        {
            const sp<Renderer> layer = factory.build<Renderer>(i, args);
            if(layer)
                arena->addLayer(layer);
            else
                arena->addLayer(factory.ensureDecorated<Renderer, RenderLayer>(i));
        }
        else if(name == Constants::Attributes::LAYER)
            arena->addLayer(factory.ensureDecorated<Renderer, Layer>(i));
        else
        {
            DWARN(name == Constants::Attributes::RENDERER || name == Constants::Attributes::RENDER_LAYER || name == Constants::Attributes::LAYER || name == Constants::Attributes::VIEW, "[Renderer, RenderLayer, Layer, View] expected, \"%s\" found", name.c_str());
            arena->addRenderer(factory.ensure<Renderer>(i, args));
        }
    }
    return arena;
}

Arena::STYLE::STYLE(BeanFactory& factory, const sp<Builder<Arena>>& delegate, const String& value)
    : _delegate(delegate), _expired(factory.ensureBuilder<Disposed>(value))
{
}

sp<Arena> Arena::STYLE::build(const sp<Scope>& args)
{
    sp<Arena> bean = _delegate->build(args);
    return bean.absorb<Disposed>(_expired->build(args));
}

}
