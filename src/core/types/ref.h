#pragma once

#include "core/base/api.h"

namespace ark {

template<typename T> class Ref {
public:
    Ref(T& instance)
        : _instance(instance), _discarded(false) {
    }

    explicit operator bool() const {
        return !_discarded;
    }

    void discard() {
        _discarded = true;
    }

    T& instance() {
        ASSERT(!_discarded);
        return _instance;
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
