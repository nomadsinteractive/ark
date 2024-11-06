#include "core/base/named_hash.h"

#include "core/types/global.h"

namespace ark {

namespace {

struct HashNames {
    std::unordered_map<HashId, String> _hash_names;

    String findName(HashId id) const
    {
        if(const auto iter = _hash_names.find(id); iter != _hash_names.end())
            return iter->second;
        return "<Unknow>";
    }
};

}

NamedHash::NamedHash(String name)
    : _hash(name.hash()), _name(std::move(name))
{
#ifdef ARK_FLAG_DEBUG
    Global<HashNames>()->_hash_names[_hash] = _name;
#endif
}

NamedHash::NamedHash(HashId hash)
    : _hash(hash)
{
}

const String& NamedHash::name() const
{
    return _name;
}

HashId NamedHash::hash() const
{
    return _hash;
}

String NamedHash::reverse(HashId hash)
{
    return Global<HashNames>()->findName(hash);
}

}
