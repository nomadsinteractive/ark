#include "app/traits/behaviour.h"

#include "core/ark.h"
#include "core/util/holder_util.h"

namespace ark {

Behaviour::Behaviour(Box delegate)
    : _delegate(std::move(delegate))
{
}

void Behaviour::traverse(const Visitor& visitor)
{
    visitor(_delegate);
}

}
