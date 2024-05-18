#include "entity.h"

namespace ark {

Entity::Entity(sp<ResourceLoader> resourceLoader)
    : _resource_loader(std::move(resourceLoader)) {
}

void Entity::addComponent(TypeId typeId, Box box)
{
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

}