#pragma once

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class Wrapper {
public:
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Wrapper);

    const sp<T>& wrapped() const {
        return _wrapped;
    }

    sp<T> reset(sp<T> wrapped) {
        sp<T> oldValue = std::move(_wrapped);
        _wrapped = std::move(wrapped);
        return oldValue;
    }

protected:
    Wrapper(sp<T> delegate) noexcept
        : _wrapped(std::move(delegate)) {
    }

protected:
    sp<T> _wrapped;
};

}
