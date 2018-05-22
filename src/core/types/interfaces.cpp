#include "core/types/interfaces.h"

#include "core/types/class.h"
#include "core/types/box.h"

namespace ark {

Interfaces::Interfaces(Class* clazz)
    : _class(clazz)
{
    NOT_NULL(_class);
}

TypeId Interfaces::typeId() const
{
    return _class->id();
}

const std::unordered_set<TypeId>& Interfaces::implements() const
{
    return _class->implements();
}

Box Interfaces::as(const Box& box, TypeId id) const
{
    const Box inst = _class->cast(box, id);
    if(inst)
        return inst;
    auto iter = _attachments.find(id);
    return iter != _attachments.end() ? iter->second : Box();
}

bool Interfaces::is(TypeId id) const
{
    return _class->id() == id;
}

bool Interfaces::isInstance(TypeId id) const
{
    return is(id) || _class->isInstance(id) || _attachments.find(id) != _attachments.end();
}

void Interfaces::reset()
{
    _attachments.clear();
}

}
