#pragma once

#include "core/base/api.h"
#include "core/util/log.h"

namespace ark {

class ARK_API Ref {
public:
    template<typename T> Ref(T& instance)
        : _discarded(false), _instance(&instance), _instance_ref(*this) {
    }
    Ref(const Ref& other);
    ~Ref();

    explicit operator bool() const;

    template<typename T> T& instance() const {
        ASSERT(!isDiscarded());
        return *static_cast<T*>(_instance);
    }

    bool isDiscarded() const;
//  [[script::bindings::auto]]
    void discard();

//  [[script::bindings::operator(index)]]
    uintptr_t toInt() const;
    static Ref& toRef(uintptr_t id);

    static uintptr_t toInteger(const Ref& self);

private:
    bool _discarded;
    void* _instance;
    const Ref& _instance_ref;
};

}
