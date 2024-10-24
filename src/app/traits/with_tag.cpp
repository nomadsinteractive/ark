#include "app/traits/with_tag.h"

namespace ark {

WithTag::WithTag(Box tag)
    : _info(std::move(tag))
{
}

const Box& WithTag::tag() const
{
    return _info;
}

}
