#include "app/components/crate.h"

#include "core/types/box.h"

namespace ark {

Crate::Crate()
{
}

void Crate::setComponent(const uint64_t typeId, Box component)
{
    _components[typeId] = std::move(component);
}

Box Crate::getComponent(const uint64_t typeId) const
{
    const auto iter = _components.find(typeId);
    return iter != _components.end() ? iter->second : Box();
}

void Crate::traverse(const Visitor& visitor)
{
    for(const auto& [k, v] : _components)
        visitor(v);
}

}
