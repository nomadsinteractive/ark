#include "entity.h"

#include <ranges>

#include "core/components/discarded.h"
#include "core/inf/wirable.h"

#include "core/base/constants.h"
#include "core/base/ref_manager.h"
#include "core/components/tags.h"
#include "core/components/with_id.h"
#include "core/types/global.h"

namespace ark {

namespace {

class RefDiscarded final : public Boolean {
public:
    RefDiscarded(sp<Ref> ref)
        : _ref(std::move(ref))
    {
    }

    bool update(const uint64_t timestamp) override
    {
        return _ref->discarded().update(timestamp);
    }

    bool val() override
    {
        return _ref->isDiscarded();
    }

private:
    sp<Ref> _ref;
};

bool containsWirable(const Vector<Box>& components)
{
    for(const Box& i : components)
        if(const sp<Wirable> wirable = i.as<Wirable>())
            return true;
    return false;
}

}

Entity::Entity(Traits components)
    : _ref(Global<RefManager>()->makeRef(this)), _components(std::move(components))
{
    Wirable::WiringContext context(_components);
    for(const auto& [k, v] : _components.table())
        if(!v.isEnum())
            if(const sp<Wirable> wirable = v.as<Wirable>())
                wirable->onPoll(context);

    preWire();
    for(const auto& [k, v] : _components.table())
        if(!v.isEnum())
            if(const sp<Wirable> wirable = v.as<Wirable>())
                wirable->onWire(context, v);
}

Entity::Entity(Vector<Component> components)
    : _ref(Global<RefManager>()->makeRef(this))
{
    Vector<sp<Wirable::Niche>> niches;
    {
        Wirable::WiringContext context(_components);
        for(const auto& [k, v] : components)
        {
            _components.add(k.typeId(), k);
            if(!k.isEnum())
            {
                if(sp<Wirable::Niche> niche = k.as<Wirable::Niche>())
                    niches.push_back(std::move(niche));
                if(const sp<Wirable> wirable = k.as<Wirable>())
                    wirable->onPoll(context);
            }
        }
    }

    preWire();
    for(const auto& [k, v] : components)
        if(!k.isEnum())
            if(const sp<Wirable> wirable = k.as<Wirable>())
            {
                Wirable::WiringContext nicheContext(_components, false);
                if(v)
                    for(const sp<Wirable::Niche>& i : niches)
                        i->onPoll(nicheContext, v);

                wirable->onWire(nicheContext, k);
            }
}

Entity::~Entity()
{
    discard();
}

void Entity::traverse(const Visitor& visitor)
{
    for(const auto& [k, v] : _components.table())
        if(const sp<Debris>& holder = v.as<Debris>())
            holder->traverse(visitor);
}

RefId Entity::id() const
{
    return _ref->id();
}

const sp<Ref>& Entity::ref() const
{
    return _ref;
}

void Entity::discard()
{
    _ref->discard();
    _components.table().clear();
}

void Entity::addComponent(Box component)
{
    setComponent(component.typeId(), std::move(component));
}

void Entity::addComponents(Vector<Box> components)
{
    {
        Traits traits;
        for(const Box& i : components)
            traits.put(i.typeId(), i);
        Wirable::WiringContext context(traits, false);
        for(const Box& i : components)
            if(const sp<Wirable> wirable = i.as<Wirable>())
                wirable->onPoll(context);
        for(const Box& i : _components.table().values())
            if(const sp<Wirable> w = i.isEnum() ? sp<Wirable>() : i.as<Wirable>())
                w->onWire(context, i);
    }
    if(containsWirable(components))
    {
        Wirable::WiringContext context(_components);
        for(const Box& i : _components.table().values())
            if(const sp<Wirable> w = i.isEnum() ? sp<Wirable>() : i.as<Wirable>())
                w->onPoll(context);
        for(const Box& i : components)
            if(const sp<Wirable> wirable = i.as<Wirable>())
                wirable->onPoll(context);
        for(const Box& i : components)
            if(const sp<Wirable> wirable = i.as<Wirable>())
                wirable->onWire(context, i);
    }
    for(Box& i : components)
        _components.add(i.typeId(), std::move(i));
}

bool Entity::hasComponent(const TypeId typeId) const
{
    return _components.has(typeId);
}

Optional<Box> Entity::getComponent(const TypeId typeId) const
{
    return _components.get(typeId);
}

void Entity::setComponent(const TypeId typeId, Box component)
{
    {
        Traits traits;
        traits.put(typeId, component);
        Wirable::WiringContext context(traits, false);
        if(const sp<Wirable> wirable = component.as<Wirable>())
            wirable->onPoll(context);
        for(const Box& i : _components.table().values())
            if(const sp<Wirable> w = i.isEnum() ? sp<Wirable>() : i.as<Wirable>())
                w->onWire(context, i);
    }
    if(const sp<Wirable> wirable = component.as<Wirable>())
    {
        Wirable::WiringContext context(_components);
        for(const Box& i : _components.table().values())
            if(const sp<Wirable> w = i.isEnum() ? sp<Wirable>() : i.as<Wirable>())
                w->onPoll(context);

        wirable->onPoll(context);
        wirable->onWire(context, component);
    }
    _components.add(typeId, std::move(component));
}

Vector<Box> Entity::getComponentList(const TypeId typeId) const
{
    Optional<Box> vectorTrait = _components.get(toVectorTypeId(typeId));
    if(!vectorTrait)
    {
        Optional<Box> cmp = _components.get(typeId);
        CHECK(cmp, "Entity has no component list for type: %d", Class::ensureClass(typeId)->name());
        return {std::move(cmp.value())};
    }
    return vectorTrait.value().toPtr<Vector<Box>>();
}

Box Entity::tag() const
{
    if(const sp<Tags> tags = _components.get<Tags>())
        return tags->tag();
    return {};
}

const Vector<Box>& Entity::components() const
{
    return _components.table().values();
}

struct Entity::BUILDER::ComponentBuilder {

    ComponentBuilder(BeanFactory& beanFactory, document manifest)
        : _wirable(beanFactory.findBuilderByDocument<sp<Wirable>>(manifest, manifest->name(), false)), _manifest(std::move(manifest))
    {
        CHECK(_wirable, "Unable to build component from \"%s\"", Documents::toString(_manifest).c_str());
    }

    builder<Wirable> _wirable;
    document _manifest;
};

Entity::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _boxes(factory.getBuilder<Vector<Box>>(manifest, "components")), _components(factory.makeBuilderListObject<ComponentBuilder>(manifest, ""))
{
}

sp<Entity> Entity::BUILDER::build(const Scope& args)
{
    Vector<Component> components;
    if(_boxes)
    {
        const sp<Vector<Box>> boxes = _boxes->build(args);
        for(Box& i : *boxes)
            if(i)
                components.push_back({std::move(i)});
    }
    for(const ComponentBuilder& i : _components)
        if(sp<Wirable> wirable = i._wirable->build(args))
        {
            const Box trait(std::move(wirable));
            components.push_back({trait.cast(trait.getClass()->id()), i._manifest});
        }
    return sp<Entity>::make(std::move(components));
}

void Entity::preWire()
{
    if(const sp<WithId>& withId = _components.get<WithId>())
        withId->_id = _ref->id();

    if(!_components.get<Discarded>())
        _components.put(sp<Discarded>::make(sp<Boolean>::make<RefDiscarded>(_ref)));
}

}
