#include "core/collection/by_name.h"

namespace ark {

ByName::ByName()
{
}

ByName::ByName(const ByName& other)
    : _items(other._items)
{
}

ByName::ByName(ByName&& other)
    : _items(std::move(other._items))
{
}

const ByName& ByName::operator =(const ByName& other)
{
    _items = other._items;
    return *this;
}

const ByName& ByName::operator =(ByName&& other)
{
    _items = std::move(other._items);
    return *this;
}

}
