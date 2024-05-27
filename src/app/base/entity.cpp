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

void Entity::doWire() const
{
    for(const auto& [k, v] : _components.traits())
        if(const sp<Wirable> wirable = v.as<Wirable>())
            wirable->onWire(_components);
}

uintptr_t Entity::id() const
{
    return reinterpret_cast<uintptr_t>(_ref.get());
}

void Entity::dispose()
{
    _ref->_discarded = true;
}

sp<RenderObject> Entity::renderObject() const {
    return _components.get<RenderObject>();
}

void Entity::addComponent(Box box)
{
    TypeId typeId = box.typeId();
    if(const sp<Wirable> wirable = box.as<Wirable>())
        wirable->onWire(_components);
    _components.put(typeId, std::move(box));
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

}