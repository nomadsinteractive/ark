#include "entity_id.h"

namespace ark {

EntityId::EntityId(const Entity& entity)
    : _ref(entity._ref)
{
}

uintptr_t EntityId::toInteger(const EntityId& self)
{
    return reinterpret_cast<uintptr_t>(self._ref.get());
}

}
