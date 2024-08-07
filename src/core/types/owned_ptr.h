#pragma once

#include "core/base/api.h"
#include "core/forwarding.h"

namespace ark {

template<typename T> class OwnedPtr {
public:
    constexpr OwnedPtr(std::nullptr_t null) noexcept
        : _inst(null), _auto_release(false) {
    }
    OwnedPtr(T* instance = nullptr, bool autoRelease = true) noexcept
        : _inst(instance), _auto_release(autoRelease) {
    }
    OwnedPtr(OwnedPtr&& other)
        : _inst(other._inst), _auto_release(other._auto_release) {
        other._inst = nullptr;
    }

    T* operator ->() const {
        return _inst;
    }

    T* get() const {
        return _inst;
    }

    template<typename U> U* cast() const {
        return static_cast<U*>(_inst);
    }

    explicit operator bool () const {
        return _inst != nullptr;
    }

    operator T& () const {
        return *_inst;
    }

    operator const T* () const {
        return _inst;
    }

    operator T* () {
        return _inst;
    }

    void reset(T* instance = nullptr, bool autoRelease = true) {
        if(_auto_release && _inst)
            delete _inst;
        _inst = instance;
        _auto_release = autoRelease;
    }

    ~OwnedPtr() {
        reset();
    }

private:
    DISALLOW_COPY_AND_ASSIGN(OwnedPtr);

    T* _inst;
    bool _auto_release;
};

}
