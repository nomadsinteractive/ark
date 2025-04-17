#include "app/components/tags.h"

#include "core/types/box.h"

namespace ark {

void Tags::setTag(Box tag, const uint64_t typeId)
{
    _tags[typeId] = std::move(tag);
}

Box Tags::getTag(const uint64_t typeId) const
{
    if(const auto iter = _tags.find(typeId); iter != _tags.end())
        return iter->second;
    return {};
}

void Tags::traverse(const Visitor& visitor)
{
    for(const auto& [k, v] : _tags)
        visitor(v);
}

}
