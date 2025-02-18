#pragma once

#include <optional>

#include "core/base/api.h"

namespace ark {

template<typename T> class Optional {
public:
    constexpr Optional() noexcept = default;
    Optional(T value) noexcept
        : _stub{{std::move(value)}} {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Optional);

    typedef std::remove_reference_t<T> OPT_TYPE;

    explicit operator bool() const {
        return static_cast<bool>(_stub);
    }

    OPT_TYPE& value() {
        return *get();
    }

    const OPT_TYPE& value() const {
        return *get();
    }

    OPT_TYPE* operator ->() {
        return get();
    }

    const OPT_TYPE* operator ->() const {
        return get();
    }

    const OPT_TYPE* get() const {
        CHECK(_stub, "Bad optional access");
        return &_stub.value()._value;
    }

    OPT_TYPE* get() {
        CHECK(_stub, "Bad optional access");
        return &_stub.value()._value;
    }

private:
    struct Stub {
        T _value;
    };

private:
    std::optional<Stub> _stub;
};

}
