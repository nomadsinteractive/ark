#pragma once

#include "core/forwarding.h"
#include "core/inf/variable.h"
#include "core/types/weak_ptr.h"

namespace ark {

template<typename T> class BooleanByWeakRef final : public Boolean {
public:
    BooleanByWeakRef(sp<T> ptr, int32_t useCount)
        : _weak_ref(std::move(ptr)), _use_count(useCount) {
    }

    bool val() override {
        return _weak_ref.useCount() <= _use_count;
    }

    bool update(uint32_t /*tick*/) override {
        return true;
    }

private:
    WeakPtr<T> _weak_ref;
    int32_t _use_count;
};

}
