#include "app/view/arena.h"

#include "core/base/bean_factory.h"
#include "core/util/bean_utils.h"
#include "core/util/holder_util.h"
#include "core/util/log.h"

#include "graphics/base/layer.h"
#include "graphics/base/render_layer.h"

#include "app/base/application_context.h"
#include "app/impl/event_listener/event_listener_list.h"


namespace ark {

Arena::Arena(const sp<ViewGroup>& view, const sp<ResourceLoader>& resourceLoader)
    : _event_listeners(new EventListenerList()), _view_group(view), _resource_loader(resourceLoader)
{
    DCHECK(_view_group.is<ViewGroup>(), "Arena's renderer delegate must be ViewGroup");
}

Arena::~Arena()
{
    Ark::instance().applicationContext()->deferUnref(std::move(_view_group));
    LOGD("");
}

void Arena::addRenderer(const sp<Renderer>& renderer)
{
    DASSERT(_view_group);
    _view_group->addRenderer(renderer);
}

void Arena::render(RenderRequest& renderRequest, const V3& position)
{
    DASSERT(_view_group);
    _view_group->render(renderRequest, position);
    for(const sp<Renderer>& i : _layers.update(renderRequest.timestamp()))
        i->render(renderRequest, position);
    for(const sp<Renderer>& i : _render_layers.update(renderRequest.timestamp()))
        i->render(renderRequest, position);
}

bool Arena::onEvent(const Event& event)
{
    DASSERT(_view_group);
    return _view_group->onEvent(event, 0.0f, 0.0f, true) || _event_listeners->onEvent(event);
}

void Arena::traverse(const Holder::Visitor& visitor)
{
    HolderUtil::visit(_view_group, visitor);
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

void Arena::addEventListener(sp<EventListener> eventListener, int32_t priority)
{
    _event_listeners->addEventListener(std::move(eventListener), priority);
}

void Arena::addLayer(sp<Renderer> layer)
{
    _layers.push_back(std::move(layer));
}

void Arena::addRenderLayer(sp<Renderer> renderLayer)
{
    _render_layers.push_back(std::move(renderLayer));
}

void Arena::setView(const sp<Renderer>& view)
{
    DCHECK(view.is<ViewGroup>(), "Arena's renderer delegate must be ViewGroup");
    _view_group = view.as<ViewGroup>();
}

const sp<ViewGroup>& Arena::view() const
{
    return _view_group;
}

Arena::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _factory(factory), _manifest(manifest), _resource_loader(factory.getBuilder<ResourceLoader>(manifest, "resource-loader")),
      _layout(factory.getBuilder<Layout>(manifest, Constants::Attributes::LAYOUT)),
      _layout_param(factory.ensureBuilder<LayoutParam>(manifest)),
      _background(Documents::getAttribute(manifest, Constants::Attributes::BACKGROUND)),
      _view(Documents::getAttribute(manifest, Constants::Attributes::VIEW))
{
}

sp<Arena> Arena::BUILDER::build(const Scope& args)
{
    const sp<ResourceLoader> r1 = _resource_loader->build(args);
    const sp<ResourceLoader> resourceLoader = r1 ? r1 : sp<ResourceLoader>::make(_factory);
    const sp<Renderer> background = _background.empty() ? nullptr : resourceLoader->load<Renderer>(_background, args);
    const sp<Renderer> view = _view ? resourceLoader->load<Renderer>(_view, args) : nullptr;
    const sp<Arena> arena = sp<Arena>::make(view ? view : sp<ViewGroup>::make(background, _layout->build(args), nullptr, _layout_param->build(args)).cast<Renderer>(), resourceLoader);

    BeanFactory& factory = resourceLoader->beanFactory();
    for(const document& i : _manifest->children())
    {
        const String& name = i->name();
        if(name == Constants::Attributes::EVENT_LISTENER)
            arena->addEventListener(factory.ensure<EventListener>(i, args));
        else if(name == Constants::Attributes::RENDER_LAYER)
            arena->addRenderLayer(factory.ensureDecorated<Renderer, RenderLayer>(i, args));
        else if(name == Constants::Attributes::LAYER)
            arena->addLayer(factory.ensureDecorated<Renderer, Layer>(i, args));
        else
        {
            DWARN(name == Constants::Attributes::RENDERER || name == Constants::Attributes::VIEW, "[Renderer, RenderLayer, Layer, View] expected, \"%s\" found", name.c_str());
            arena->addRenderer(factory.ensure<Renderer>(i, args));
        }
    }
    return arena;
}

}
