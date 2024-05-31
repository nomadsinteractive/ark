#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/class.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class Ref {
public:
    Ref(T& instance)
        : _instance(instance), _discarded(false) {
    }
    DISALLOW_COPY_AND_ASSIGN(Ref);

    explicit operator bool() const {
        return !_discarded;
    }

    bool isDiscarded() const {
        return _discarded;
    }

    void discard() {
        _discarded = true;
    }

    template<typename U = T> U& instance() {
        ASSERT(!_discarded);
        return static_cast<U&>(_instance);
    }

    template<typename U = T> const U& instance() const {
        ASSERT(!_discarded);
        return static_cast<const U&>(_instance);
    }

    uintptr_t id() const {
        return reinterpret_cast<uintptr_t>(this);
    }

    static Ref& toRef(uintptr_t id) {
        return *reinterpret_cast<Ref*>(id);
    }

private:
    T& _instance;
    bool _discarded;
};

}
