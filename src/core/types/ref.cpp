#include "core/types/ref.h"

#include "core/base/ref_manager.h"
#include "core/types/global.h"

namespace ark {

Ref::Ref(const RefId id, void* instance, sp<Boolean> discarded)
    : _id(id), _instance(instance), _discarded(std::move(discarded), false)
{
}

Ref::~Ref()
{
    if(_id)
        Global<RefManager>()->recycle(_id);
}

Ref::operator bool() const
{
    return !isDiscarded();
}

RefId Ref::id() const
{
    return _id;
}

bool Ref::isDiscarded() const
{
    return _discarded.val();
}

const SafeVar<Boolean>& Ref::discarded() const
{
    return _discarded;
}

void Ref::setDiscarded(sp<Boolean> discarded)
{
    _discarded.reset(std::move(discarded));
}

void Ref::discard()
{
    _discarded.reset(true);
}

}
