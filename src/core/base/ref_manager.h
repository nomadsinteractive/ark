#pragma once

#include <vector>

#include "core/types/owned_ptr.h"
#include "core/types/ref.h"
#include "core/concurrent/lf_stack.h"

namespace ark {

class RefManager {
public:

    sp<Ref> makeRef(void* instance, sp<Boolean> discarded = nullptr);
    Ref& toRef(IdType id);

private:
    void recycle(void* ref);

    struct RefSlot {
        op<Ref> _ref;
        bool _allocated;
    };

private:
    std::vector<RefSlot> _ref_slots;
    LFStack<uint32_t> _recycled_ids;
};

}
