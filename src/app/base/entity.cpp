#include "entity.h"

#include "core/inf/wirable.h"

#include "core/base/constants.h"
#include "core/base/ref_manager.h"
#include "core/traits/with_id.h"
#include "core/types/global.h"
#include "core/util/wirable_type.h"

namespace ark {

Entity::Entity(Traits components)
    : _ref(Global<RefManager>()->makeRef(this)), _components(std::move(components))
{
    doWire();
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

void Entity::doWire()
{
    if(const sp<WithId>& withId = _components.get<WithId>())
        withId->_id = _ref->id();

    WirableType::wireAll(_components);
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

void Entity::addComponent(Box component)
{
    Wirable::WiringContext context(_components);

    if(const sp<Wirable> wirable = component.as<Wirable>())
        if(const TypeId typeId = wirable->onPoll(context); typeId != constants::TYPE_ID_NONE)
            _components.put(typeId, component);
    if(const sp<Wirable> wirable = component.as<Wirable>())
        wirable->onWire(context);

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

const Traits& Entity::components() const
{
    return _components;
}

Entity::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _boxes(factory.getBuilder<std::vector<Box>>(manifest, "components")), _components(factory.makeBuilderList<Wirable>(manifest, "component"))
{
}

sp<Entity> Entity::BUILDER::build(const Scope& args)
{
    Traits traits;
    if(_boxes)
    {
        const sp<std::vector<Box>> boxes = _boxes->build(args);
        for(Box& i : *boxes)
            traits.put(i.typeId(), std::move(i));
    }
    for(const sp<Builder<Wirable>>& i : _components)
    {
        Box trait(i->build(args));
        traits.put(trait.getClass()->id(), std::move(trait));
    }
    return sp<Entity>::make(std::move(traits));
}

}
