#include "core/base/named_hash.h"

#include "core/types/global.h"

namespace ark {

namespace {

struct HashNames {
    std::unordered_map<HashId, String> _hash_names;

    const String& findName(HashId id) const
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
#ifdef ARK_FLAG_DEBUG
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

const String& NamedHash::reverse(const HashId hash)
{
    return Global<HashNames>()->findName(hash);
}

}
