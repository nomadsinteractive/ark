#ifndef ARK_CORE_BASE_DELEGATE_H_
#define ARK_CORE_BASE_DELEGATE_H_

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class Delegate {
public:
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Delegate);

    const sp<T>& delegate() const {
        return _delegate;
    }

    void reset(sp<T> delegate) {
        _delegate = std::move(delegate);
    }

protected:
    Delegate(sp<T> delegate)
        : _delegate(std::move(delegate)) {
    }

protected:
    sp<T> _delegate;
};

}

#endif
