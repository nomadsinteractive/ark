#ifndef ARK_CORE_TYPES_LAZY_PTR_H_
#define ARK_CORE_TYPES_LAZY_PTR_H_

#include <functional>

#include "core/types/null.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T, typename IMPL = T> class SafePtr {
public:
    SafePtr()
        : _inst(Null::ptr<T>()), _allocated(false) {
    }
    SafePtr(const sp<T>& inst)
        : _inst(Null::toSafe<T>(inst)), _allocated(inst) {
    }
    SafePtr(const SafePtr& other)
        : _inst(other._inst), _allocated(other._allocated) {
    }
    SafePtr(SafePtr&& other)
        : _inst(std::move(other._inst)), _allocated(other._allocated) {
    }

    explicit operator bool() const {
        return _allocated;
    }

    T* operator ->() const {
        return _inst.get();
    }

    void assign(const sp<T>& inst) {
        _inst = Null::toSafe<T>(inst);
        _allocated = static_cast<bool>(inst);
    }

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
