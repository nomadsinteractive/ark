#include "core/components/tags.h"

#include <ranges>

#include "core/types/box.h"

namespace ark {

Tags::Tags(Box tag, Args tags)
{
    if(tag)
        setTag({}, std::move(tag));

    for(Box& i : tags._values)
        setTag(i.typeId(), std::move(i));
}

Box Tags::tag() const
{
    return getTag({});
}

void Tags::addTag(const TypeId typeId)
{
    _tags[typeId] = Box(typeId);
}

bool Tags::hasTag(const TypeId typeId) const
{
    return _tags.contains(typeId);
}

void Tags::removeTag(const TypeId typeId)
{
    ASSERT(hasTag(typeId));
    delTag(typeId);
}

void Tags::setTag(const TypeId typeId, Box tag)
{
    _tags[typeId] = std::move(tag);
}

void Tags::delTag(const TypeId typeId)
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
