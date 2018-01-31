#include "core/types/interfaces.h"

#include "core/types/box.h"

namespace ark {

Box Interfaces::as(TypeId id) const
{
    auto iter = _types.find(id);
    return iter != _types.end() ? iter->second->as(id) : Box();
}

bool Interfaces::isInstance(TypeId id) const
{
    return _types.find(id) != _types.end();
}

void Interfaces::reset()
{
    _types.clear();
}

}
