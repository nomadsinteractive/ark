#include "core/base/named_type.h"

#include "core/types/global.h"

namespace ark {

namespace {

struct HashNames {
    std::unordered_map<HashId, String> _hash_names;

    String findName(HashId hash) const
    {
        const auto iter = _hash_names.find(hash);
        if(iter != _hash_names.end())
            return iter->second;
        return "<Unknow>";
    }
};

}

NamedType::NamedType(String name)
    : _id(name.hash()), _name(std::move(name))
{
#ifdef ARK_FLAG_DEBUG
    Global<HashNames>()->_hash_names[_id] = _name;
#endif
}

NamedType::NamedType(TypeId typeId)
    : _id(typeId)
{
}

bool NamedType::update(uint64_t timestamp)
{
    return _timestamp.update(timestamp);
}

int32_t NamedType::val()
{
    return _id;
}

const String& NamedType::name() const
{
    return _name;
}

TypeId NamedType::id() const
{
    return _id;
}

String NamedType::reverse(int32_t type)
{
    return Global<HashNames>()->findName(type);
}

}
