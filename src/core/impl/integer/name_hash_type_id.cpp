#include "core/impl/integer/name_hash_type_id.h"

namespace ark {

NameHashTypeId::NameHashTypeId(String name)
    : _name(std::move(name)), _name_hash(string_hash(_name.c_str()))
{
}

bool NameHashTypeId::update(uint64_t /*timestamp*/)
{
    return false;
}

int32_t NameHashTypeId::val()
{
    return _name_hash;
}

const String& NameHashTypeId::name() const
{
    return _name;
}

}
