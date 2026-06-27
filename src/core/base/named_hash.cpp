#include "core/base/named_hash.h"

#include "core/types/global.h"

namespace ark {

namespace {

struct HashNames {
    HashMap<HashId, String> _hash_names;

    const String& findName(const HashId id) const
    {
        if(const auto iter = _hash_names.find(id); iter != _hash_names.end())
            return iter->second;

        static String _name_unknown = "<Unknow>";
        return _name_unknown;
    }
};

}

NamedHash::NamedHash(String value)
    : _hash_code(value.hash()), _name(std::move(value))
{
#ifndef ARK_FLAG_PUBLISHING_BUILD
    Global<HashNames>()->_hash_names[_hash_code] = _name;
#endif
}

NamedHash::NamedHash(const HashId value)
    : _hash_code(value)
{
}

NamedHash::NamedHash()
    : _hash_code(0)
{
}

NamedHash::operator bool() const
{
    return static_cast<bool>(_hash_code);
}

const String& NamedHash::name() const
{
    return _name ? _name : reverse(_hash_code);
}

HashId NamedHash::hashCode() const
{
    return _hash_code;
}

bool NamedHash::update(uint32_t /*tick*/)
{
    return false;
}

int32_t NamedHash::val()
{
    return _hash_code;
}

bool NamedHash::operator==(const NamedHash& other) const
{
    return _hash_code == other._hash_code;
}

bool NamedHash::operator<(const NamedHash& other) const
{
    return _hash_code < other._hash_code;
}

const String& NamedHash::reverse(const HashId hash)
{
    return Global<HashNames>()->findName(hash);
}

}
