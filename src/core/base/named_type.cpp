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
    : _name(std::move(name)), _type(_name.hash())
{
#ifdef ARK_FLAG_DEBUG
    Global<HashNames>()->_hash_names[_type] = _name;
#endif
}

NamedType::NamedType(int32_t type)
    : _type(type)
{
}

bool NamedType::update(uint64_t timestamp)
{
    return _timestamp.update(timestamp);
}

int NamedType::val()
{
    return _type;
}

const String& NamedType::name() const
{
    return _name;
}

int32_t NamedType::type() const
{
    return _type;
}

String NamedType::reverse(int32_t type)
{
    return Global<HashNames>()->findName(type);
}

}
