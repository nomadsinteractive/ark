#include "core/collection/by_type.h"

namespace ark {

ByType::ByType()
{
}

ByType::ByType(const ByType& other)
    : _items(other._items)
{
}

ByType::ByType(ByType&& other)
    : _items(std::move(other._items))
{
}

}
