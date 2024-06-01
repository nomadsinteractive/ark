#include "app/traits/tag.h"

namespace ark {

Tag::Tag(Box info)
    : _info(std::move(info))
{
}

const Box& Tag::info() const
{
    return _info;
}

}
