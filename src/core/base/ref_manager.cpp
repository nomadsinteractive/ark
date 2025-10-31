#include "core/base/ref_manager.h"

namespace ark {

RefManager::RefManager()
    : _ref_null(makeRef(nullptr, nullptr))
{
}

RefManager::~RefManager()
{
    recycle(0);
}

sp<Ref> RefManager::makeRef(void* instance, sp<Boolean> discarded)
{
    if(Optional<RefId> optRefId = _recycled_ids.pop())
    {
        WeakPtr<Ref>& wp = _ref_slots[optRefId.value()];
        DCHECK(!wp, "Ref(%d) has been already allocated while making a new Ref", optRefId.value());
        sp<Ref> ref = sp<Ref>::make(optRefId.value(), instance, std::move(discarded));
        wp = {ref};
        return ref;
    }

    sp<Ref> ref = sp<Ref>::make(static_cast<RefId>(_ref_slots.size()), instance, std::move(discarded));
    _ref_slots.emplace_back(ref);
    return ref;
}

sp<Ref> RefManager::toRef(const RefId refid) const
{
    CHECK(refid <= _ref_slots.size(), "Invaild Ref(%d)", refid);
    CHECK(!_ref_slots.at(refid).expired(), "Ref(%d) expired", refid);
    return _ref_slots.at(refid).ensure();
}

void RefManager::recycle(const RefId refid)
{
    CHECK(refid <= _ref_slots.size(), "Ref(%d) has not been allocated", refid);
    _ref_slots[refid] = {};
    _recycled_ids.push(refid);
}

}
