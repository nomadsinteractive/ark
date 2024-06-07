#pragma once

#include "core/base/api.h"

namespace ark {

template<typename T> class Ref {
public:
    Ref(T& instance)
        : _instance(instance), _discarded(false), _origin(*this) {
    }
    Ref(const Ref& other)
        : _instance(other._instance), _discarded(other._discarded), _origin(other) {
    }

    explicit operator bool() const {
        return !isDiscarded();
    }

    bool isDiscarded() const {
        return _origin._discarded;
    }

    void discard() {
        _discarded = true;
    }

    template<typename U = T> U& instance() {
        ASSERT(!isDiscarded());
        return static_cast<U&>(_instance);
    }

    template<typename U = T> const U& instance() const {
        ASSERT(!isDiscarded());
        return static_cast<const U&>(_instance);
    }

    uintptr_t id() const {
        return reinterpret_cast<uintptr_t>(&_origin);
    }

    static Ref& toRef(uintptr_t id) {
        return *reinterpret_cast<Ref*>(id);
    }

private:
    T& _instance;
    bool _discarded;
    const Ref& _origin;
};

}
