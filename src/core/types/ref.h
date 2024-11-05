#pragma once

#include "core/base/api.h"
#include "core/types/safe_var.h"
#include "core/util/log.h"

namespace ark {

class ARK_API Ref {
public:
    ~Ref();
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Ref);

    explicit operator bool() const;

    template<typename T> T& instance() const {
        ASSERT(!isDiscarded());
        return *static_cast<T*>(_instance);
    }

//  [[script::bindings::operator(index)]]
//  [[script::bindings::property]]
    RefId id() const;

    bool isDiscarded() const;
//  [[script::bindings::property]]
    const SafeVar<Boolean>& discarded() const;
//  [[script::bindings::property]]
    void setDiscarded(sp<Boolean> discarded);

//  [[script::bindings::auto]]
    void discard();

private:
    Ref(RefId id, void* instance, sp<Boolean> discarded = nullptr);

private:
    RefId _id;
    void* _instance;
    SafeVar<Boolean> _discarded;

    friend class RefManager;
};

}
