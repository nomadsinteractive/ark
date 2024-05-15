#include "core/types/interfaces.h"

#include "core/types/class.h"
#include "core/types/box.h"

namespace ark {

Interfaces::Interfaces(Class* clazz)
    : _class(clazz)
{
    DASSERT(_class);
}

void Interfaces::absorb(const Box& other)
{
    for(auto i : other.interfaces()->implements())
        _attachments[i] = other.interfaces()->as(other, i);
    _attachments.insert(other.interfaces()->_attachments.begin(), other.interfaces()->_attachments.end());
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
