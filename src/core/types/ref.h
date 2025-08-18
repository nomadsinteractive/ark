#pragma once

#include "core/base/api.h"
#include "core/types/optional_var.h"

namespace ark {

class ARK_API Ref {
public:
    Ref(RefId id, void* instance, sp<Boolean> discarded = nullptr);
    ~Ref();
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Ref);

    explicit operator bool() const;

    template<typename T> T& instance() const {
        ASSERT(!isDiscarded());
        return *static_cast<T*>(_instance);
    }

    RefId id() const;

    bool isDiscarded() const;
    const OptionalVar<Boolean>& discarded() const;
    void setDiscarded(sp<Boolean> discarded);

    void discard();

private:
    RefId _id;
    void* _instance;
    OptionalVar<Boolean> _discarded;

    friend class RefManager;
};

}
