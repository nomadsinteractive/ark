#include "core/base/named_type.h"

namespace ark {

NamedType::NamedType(String name)
    : _name(std::move(name)), _type(_name.hash())
{
}

NamedType::NamedType(int32_t type)
    : _type(type)
{
}

const String& NamedType::name() const
{
    return _name;
}

int32_t NamedType::type() const
{
    return _type;
}

}
