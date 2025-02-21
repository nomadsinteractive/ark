#include "entity.h"

#include "core/components/discarded.h"
#include "core/components/with_tag.h"
#include "core/inf/wirable.h"

#include "core/base/constants.h"
#include "core/base/ref_manager.h"
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

    bool update(uint64_t timestamp) override
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

}

Entity::Entity(Traits components)
    : _ref(Global<RefManager>()->makeRef(this)), _components(std::move(components))
{
    Wirable::WiringContext context(_components);
    for(const auto& [k, v] : _components.traits())
        if(const sp<Wirable> wirable = v.as<Wirable>())
            if(const TypeId typeId = wirable->onPoll(context); typeId != constants::TYPE_ID_NONE)
                _components.add(typeId, v);

    preWire();
    for(const auto& [k, v] : _components.traits())
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
            _components.put(k.typeId(), k);
            if(sp<Wirable::Niche> niche = k.as<Wirable::Niche>())
                niches.push_back(std::move(niche));
            if(const sp<Wirable> wirable = k.as<Wirable>())
                if(const TypeId typeId = wirable->onPoll(context); typeId != constants::TYPE_ID_NONE)
                    _components.put(typeId, k);
        }
    }

    preWire();
    for(const auto& [k, v] : components)
        if(const sp<Wirable> wirable = k.as<Wirable>())
        {
            Wirable::WiringContext nicheContext(_components, false);
            if(v)
                for(const sp<Wirable::Niche>& i : niches)
                    if(const String value = Documents::getAttribute(v, i->name()))
                        i->onPoll(nicheContext, value);

            wirable->onWire(nicheContext, k);
        }
}

Entity::~Entity()
{
    discard();
}

void Entity::traverse(const Visitor& visitor)
{
    for(const auto& [k, v] : _components.traits())
        if(const sp<Debris>& holder = v.as<Debris>())
            holder->traverse(visitor);
}

const sp<Ref>& Entity::ref() const
{
    return _ref;
}

void Entity::discard()
{
    _ref->discard();
    _components.traits().clear();
}

sp<Discarded> Entity::discarded() const
{
    return _components.get<Discarded>();
}

void Entity::addComponent(Box component)
{
    Wirable::WiringContext context(_components);

    if(const sp<Wirable> wirable = component.as<Wirable>())
        if(const TypeId typeId = wirable->onPoll(context); typeId != constants::TYPE_ID_NONE)
            _components.put(typeId, component);
    if(const sp<Wirable> wirable = component.as<Wirable>())
        wirable->onWire(context, component);

    _components.put(component.typeId(), std::move(component));
}

bool Entity::hasComponent(TypeId typeId) const
{
    return _components.has(typeId);
}

Optional<Box> Entity::getComponent(TypeId typeId) const
{
    return _components.get(typeId);
}

Box Entity::tag() const
{
    if(const sp<WithTag> withTag = _components.get<WithTag>())
        return withTag->tag();
    return {};
}

const Traits& Entity::components() const
{
    return _components;
}

struct Entity::BUILDER::ComponentBuilder {

    ComponentBuilder(BeanFactory& beanFactory, document manifest)
        : _wirable(beanFactory.findBuilderByDocument<Wirable>(manifest, manifest->name(), false)), _manifest(std::move(manifest))
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
