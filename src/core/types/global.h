#pragma once

#include "core/ark.h"

namespace ark {

template<typename T> class Global {
public:
    Global()
        : _inst(Ark::instance().ensure<T>()) {
    }
    Global(const Global& other) = default;

    T* operator ->() const {
        return _inst.get();
    }

    operator sp<T> () const {
        return _inst;
    }

    operator const T& () const {
        return *_inst;
    }

    operator T& () {
        return *_inst;
    }

    template<typename U> sp<U> cast() const {
        return _inst.template cast<U>();
    }

private:
    sp<T> _inst;
};

}
