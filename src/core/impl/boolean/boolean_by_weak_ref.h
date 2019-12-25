#ifndef ARK_CORE_IMPL_BOOLEAN_BY_WEAK_REF_H_
#define ARK_CORE_IMPL_BOOLEAN_BY_WEAK_REF_H_

#include "core/forwarding.h"
#include "core/inf/variable.h"
#include "core/types/weak_ptr.h"

namespace ark {

template<typename T> class BooleanByWeakRef : public Boolean {
public:
    BooleanByWeakRef(const sp<T>& ptr, long useCount)
        : _weak_ref(ptr), _use_count(useCount) {
    }

    virtual bool val() override {
        return _weak_ref.useCount() <= _use_count;
    }

    virtual bool update(uint64_t /*timestamp*/) override {
        return true;
    }

private:
    WeakPtr<T> _weak_ref;
    long _use_count;

};

}

#endif
