#ifndef ARK_CORE_TYPES_OWNED_PTR_H_
#define ARK_CORE_TYPES_OWNED_PTR_H_

#include "core/base/api.h"
#include "core/forwarding.h"

namespace ark {

template<typename T> class OwnedPtr {
public:
    OwnedPtr(T* instance = nullptr, bool autoRelease = true) noexcept
        : _inst(instance), _auto_release(autoRelease) {
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

    operator bool () const {
        return _inst != nullptr;
    }

    operator const T& () const {
        return *_inst;
    }

    operator T& () const {
        return *_inst;
    }

    void reset(T* instance, bool autoRelease = true) {
        if(_auto_release && _inst)
            delete _inst;
        _inst = instance;
        _auto_release = autoRelease;
    }

    ~OwnedPtr() {
        reset(nullptr);
    }

private:
    DISALLOW_COPY_AND_ASSIGN(OwnedPtr);

    T* _inst;
    bool _auto_release;
};

}

#endif
