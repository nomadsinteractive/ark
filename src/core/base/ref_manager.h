#pragma once

#include "core/base/api.h"
#include "core/types/weak_ptr.h"
#include "core/types/ref.h"
#include "core/concurrent/lf_stack.h"

namespace ark {

class ARK_API RefManager {
public:
    RefManager();
    ~RefManager();

    sp<Ref> makeRef(void* instance, sp<Boolean> discarded = nullptr);
    sp<Ref> toRef(RefId refid) const;

    void recycle(RefId refid);

private:
    Vector<WeakPtr<Ref>> _ref_slots;
    LFStack<uint32_t> _recycled_ids;

    sp<Ref> _ref_null;
};

}
