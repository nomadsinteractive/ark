#pragma once

#include "core/base/api.h"
#include "core/types/owned_ptr.h"
#include "core/types/ref.h"
#include "core/concurrent/lf_stack.h"

namespace ark {

class ARK_API RefManager {
public:

    sp<Ref> makeRef(void* instance, sp<Boolean> discarded = nullptr);
    Ref& toRef(RefId id);

private:
    void recycle(void* ref);

    struct RefSlot {
        op<Ref> _ref;
        bool _allocated;
    };

private:
    Vector<RefSlot> _ref_slots;
    LFStack<uint32_t> _recycled_ids;
};

}
