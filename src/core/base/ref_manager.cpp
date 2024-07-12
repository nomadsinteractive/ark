#include "core/base/ref_manager.h"

namespace ark {

sp<Ref> RefManager::makeRef(void* instance, sp<Boolean> discarded)
{
    IdType refId;
    if(_recycled_ids.pop(refId))
    {
        DCHECK(!_ref_slots.at(refId)._allocated, "Ref(%d) has been already allocated while making a new Ref", refId);
        auto& [ref, allocated] = _ref_slots[refId];
        allocated = true;
        *ref = Ref(refId, instance, std::move(discarded));
    }
    else
    {
        refId = static_cast<IdType>(_ref_slots.size());
        _ref_slots.push_back({new Ref{refId, instance, std::move(discarded)}, true});
    }
    return sp<Ref>(std::shared_ptr<Ref>(_ref_slots[refId]._ref.get(), [this](void* ref) { recycle(ref); }), nullptr);
}

Ref& RefManager::toRef(IdType id)
{
    CHECK(id < _ref_slots.size() && _ref_slots.at(id)._allocated, "Invaild Ref(%d)", id);
    return _ref_slots.at(id)._ref;
}

void RefManager::recycle(void* ref)
{
    const Ref* r = static_cast<Ref*>(ref);
    DCHECK(_ref_slots.at(r->id())._allocated, "Ref(%d) has not been allocated", r->id());
    _ref_slots.at(r->id())._allocated = false;
    _recycled_ids.push(r->id());
}

}
