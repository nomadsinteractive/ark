#include "core/components/tags.h"

#include "core/types/box.h"

namespace ark {

Tags::Tags(Box tag)
{
    if(tag)
        setTag(0, std::move(tag));
}

Box Tags::tag() const
{
    return getTag(0);
}

void Tags::setTag(const uint64_t typeId, Box tag)
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
