#ifndef ARK_CORE_TYPES_GLOBAL_H_
#define ARK_CORE_TYPES_GLOBAL_H_

#include "core/ark.h"

namespace ark {

template<typename T> class Global {
public:
    Global()
        : _inst(Ark::instance().ensure<T>()) {
        DASSERT(_inst);
    }
    Global(const Global& other)
        : _inst(other._inst) {
    }

    T* operator ->() const {
        return _inst.get();
    }

    operator const sp<T>& () const {
        return _inst;
    }

    operator const T& () const {
        return _inst;
    }

private:
    const sp<T>& _inst;
};

}

#endif
