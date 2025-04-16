#include "app/components/tags.h"

#include "core/types/box.h"

namespace ark {

Tags::Tags()
{
}

void Tags::setTag(const uint64_t typeId, Box tag)
{
    _tags[typeId] = std::move(tag);
}

Box Tags::getTag(const uint64_t typeId) const
{
    const auto iter = _tags.find(typeId);
    return iter != _tags.end() ? iter->second : Box();
}

void Tags::traverse(const Visitor& visitor)
{
    for(const auto& [k, v] : _tags)
        visitor(v);
}

}
