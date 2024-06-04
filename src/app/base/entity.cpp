#include "entity.h"

#include "core/inf/wirable.h"

#include "app/base/entity_id.h"
#include "core/base/constants.h"

namespace ark {

Entity::Entity(Traits components)
    : _ref(sp<EntityRef>::make(*this)), _components(std::move(components))
{
    doWire();
}

Entity::~Entity()
{
    dispose();
}

void Entity::traverse(const Visitor& visitor)
{
    for(const auto& [k, v] : _components.traits())
        if(const sp<Holder>& holder = v.as<Holder>())
            holder->traverse(visitor);
}

void Entity::doWire()
{
    Wirable::WiringContext context(_components);
    for(const auto& [k, v] : _components.traits())
        onWireOne(context, v);
}

sp<EntityId> Entity::id() const
{
    return sp<EntityId>::make(*this);
}

void Entity::dispose()
{
    _ref->discard();
    _components.traits().clear();
}

void Entity::addComponent(Box component)
{
    Wirable::WiringContext context(_components);
    onWireOne(context, component);

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

void Entity::onWireOne(Wirable::WiringContext& wiringContext, const Box& component)
{
    if(const sp<Wirable> wirable = component.as<Wirable>())
        if(const TypeId typeId = wirable->onWire(wiringContext); typeId != TYPE_ID_NONE)
            _components.put(typeId, component);
}

}
