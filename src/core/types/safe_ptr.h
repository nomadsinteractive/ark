#ifndef ARK_CORE_TYPES_LAZY_PTR_H_
#define ARK_CORE_TYPES_LAZY_PTR_H_

#include <functional>

#include "core/types/null.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T, typename IMPL = T> class SafePtr {
public:
    SafePtr()
        : _inst(Null::ptr<T>()), _allocated(std::is_same<T, IMPL>::value) {
    }
    SafePtr(const sp<T>& inst)
        : _inst(Null::toSafe<T>(inst)), _allocated(inst || std::is_same<T, IMPL>::value) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(SafePtr);

    T* operator ->() const {
        return _inst.get();
    }

//[[deprecated]]
    const sp<T>& ensure() const {
        if(!_allocated) {
            _inst = sp<IMPL>::make(*_inst);
            _allocated = true;
        }
        return _inst;
    }

private:
    mutable sp<T> _inst;
    mutable bool _allocated;
};

}

#endif
