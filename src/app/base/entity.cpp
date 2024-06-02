#include "entity.h"

#include "core/inf/wirable.h"

#include "app/base/entity_id.h"

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
    for(const auto& [k, v] : _components.traits())
        onWireOne(v);
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
    onWireOne(component);
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

void Entity::onWireOne(const Box& component)
{
    if(const sp<Wirable> wirable = component.as<Wirable>())
        for(auto& [k, v] : wirable->onWire(_components))
            _components.put(k, v ? std::move(v) : component);
}

}