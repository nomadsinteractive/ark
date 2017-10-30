#ifndef ARK_CORE_IMPL_BOOLEAN_BY_WEAK_REF_H_
#define ARK_CORE_IMPL_BOOLEAN_BY_WEAK_REF_H_

#include "core/forwarding.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class BooleanByWeakRef : public Boolean {
public:
    BooleanByWeakRef(const sp<T>& ptr, long useCount = 0)
        : _weak_ref(ptr.ptr()), _use_count(useCount) {
    }
    BooleanByWeakRef(const BooleanByWeakRef& other)
        : _weak_ref(other._weak_ref), _use_count(other._use_count) {
    }

    virtual bool val() override {
        return _weak_ref.use_count() <= _use_count;
    }

private:
    std::weak_ptr<T> _weak_ref;
    long _use_count;
};

}

#endif
