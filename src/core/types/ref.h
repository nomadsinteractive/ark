#pragma once

#include "core/base/api.h"
#include "core/types/safe_var.h"
#include "core/util/log.h"

namespace ark {

class ARK_API Ref {
public:
    template<typename T> Ref(T& instance, sp<Boolean> discarded = nullptr)
        : _instance(&instance), _instance_ref(*this), _discarded(std::move(discarded), false) {
    }
    Ref(const Ref& other);
    ~Ref();

    explicit operator bool() const;

    template<typename T> T& instance() const {
        ASSERT(!isDiscarded());
        return *static_cast<T*>(_instance);
    }

    bool isDiscarded() const;
//  [[script::bindings::property]]
    const SafeVar<Boolean>& discarded() const;
//  [[script::bindings::property]]
    void setDiscarded(sp<Boolean> discarded);

//  [[script::bindings::auto]]
    void discard();

//  [[script::bindings::property]]
    uint32_t hash() const;

//  [[script::bindings::operator(index)]]
    uintptr_t toInt() const;
    static Ref& toRef(uintptr_t id);

    static uintptr_t toInteger(const Ref& self);

private:
    void* _instance;
    const Ref& _instance_ref;
    SafeVar<Boolean> _discarded;
};

}
