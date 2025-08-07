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
    : _hash(value.hash()), _name(std::move(value))
{
#ifndef ARK_FLAG_PUBLISHING_BUILD
    Global<HashNames>()->_hash_names[_hash] = _name;
#endif
}

NamedHash::NamedHash(const HashId value)
    : _hash(value)
{
}

NamedHash::operator bool() const
{
    return static_cast<bool>(_hash);
}

const String& NamedHash::name() const
{
    return _name ? _name : reverse(_hash);
}

HashId NamedHash::hash() const
{
    return _hash;
}

bool NamedHash::update(uint64_t /*timestamp*/)
{
    return false;
}

int32_t NamedHash::val()
{
    return _hash;
}

const String& NamedHash::reverse(const HashId hash)
{
    return Global<HashNames>()->findName(hash);
}

}
