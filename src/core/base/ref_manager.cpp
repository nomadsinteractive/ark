#include "core/base/ref_manager.h"

namespace ark {

sp<Ref> RefManager::makeRef(void* instance, sp<Boolean> discarded)
{
    RefId refId;
    if(_recycled_ids.pop(refId))
    {
        WeakPtr<Ref>& wp = _ref_slots[refId - 1];
        DCHECK(!wp, "Ref(%d) has been already allocated while making a new Ref", refId);
        sp<Ref> ref = sp<Ref>::make(refId, instance, std::move(discarded));
        wp = {ref};
        return ref;
    }
    refId = static_cast<RefId>(_ref_slots.size() + 1);
    sp<Ref> ref = sp<Ref>::make(refId, instance, std::move(discarded));
    _ref_slots.emplace_back(ref);
    return ref;
}

sp<Ref> RefManager::toRef(const RefId refid) const
{
    CHECK(refid <= _ref_slots.size() && !_ref_slots.at(refid - 1).expired(), "Invaild Ref(%d)", refid);
    return _ref_slots.at(refid - 1).ensure();
}

void RefManager::recycle(const RefId refid)
{
    CHECK(refid <= _ref_slots.size() && !_ref_slots.at(refid - 1).expired(), "Ref(%d) has not been allocated", refid);
    _ref_slots[refid - 1] = {};
    _recycled_ids.push(refid);
}

}
