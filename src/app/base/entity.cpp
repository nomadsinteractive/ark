#include "entity.h"

#include "core/inf/wirable.h"

namespace ark {

Entity::Entity(Traits components)
    : _ref(sp<Ref>::make(*this)), _components(std::move(components))
{
    doWire();
}

Entity::~Entity()
{
    dispose();
}

Entity::Ref::Ref(Entity& entity)
    : _entity(entity), _discarded(false) {
}

void Entity::doWire()
{
    for(const auto& [k, v] : _components.traits())
        onWireOne(v);
}

uintptr_t Entity::id() const
{
    return reinterpret_cast<uintptr_t>(_ref.get());
}

void Entity::dispose()
{
    _ref->_discarded = true;
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