#include "core/components/tags.h"

#include <ranges>

#include "core/types/box.h"

namespace ark {

Tags::Tags(Box tag, Vector<Box> tags)
{
    if(tag)
        setTag(0, std::move(tag));

    for(Box& i : tags)
        setTag(i.typeId(), std::move(i));
}

Box Tags::tag() const
{
    return getTag(0);
}

void Tags::setTag(const TypeId typeId, Box tag)
{
    _tags[typeId] = std::move(tag);
}

void Tags::removeTag(const TypeId typeId)
{
    if(const auto iter = _tags.find(typeId); iter != _tags.end())
        _tags.erase(iter);
}

Box Tags::getTag(const TypeId typeId) const
{
    if(const auto iter = _tags.find(typeId); iter != _tags.end())
        return iter->second;
    return {};
}

void Tags::traverse(const Visitor& visitor)
{
    for(const auto& v : _tags | std::views::values)
        visitor(v);
}

}
