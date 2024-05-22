#include "core/types/interfaces.h"

#include "core/types/class.h"
#include "core/types/box.h"

namespace ark {

Interfaces::Interfaces(Class* clazz)
    : _class(clazz)
{
    DASSERT(_class);
}

TypeId Interfaces::typeId() const
{
    return _class->id();
}

const std::set<TypeId>& Interfaces::implements() const
{
    return _class->implements();
}

Box Interfaces::as(const Box& box, TypeId id) const
{
    return _class->cast(box, id);
}

bool Interfaces::is(TypeId id) const
{
    return _class->id() == id;
}

bool Interfaces::isInstance(TypeId id) const
{
    return is(id) || _class->isInstance(id);
}

}
