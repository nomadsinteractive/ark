#ifndef ARK_CORE_TYPES_OPTIONAL_H_
#define ARK_CORE_TYPES_OPTIONAL_H_

#include <optional>

#include "core/base/api.h"

namespace ark {

template<typename T> class Optional {
public:
    _CONSTEXPR Optional() noexcept {
    }
    Optional(T value) noexcept
        : _optional(Stub(std::move(value))) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Optional);

    typedef std::remove_reference_t<T> OPT;

    explicit operator bool() const {
        return static_cast<bool>(_optional);
    }

    OPT& value() {
        return *_ptr();
    }

    const OPT& value() const {
        return *_ptr();
    }

    OPT* operator ->() {
        return _ptr();
    }

    const OPT* operator ->() const {
        return _ptr();
    }

private:
    struct Stub {
        Stub(T value)
            : _value(std::move(value)) {
        }

        T _value;
    };

    const OPT* _ptr() const {
        CHECK(_optional, "Bad optional access");
        return &_optional.value()._value;
    }

    OPT* _ptr() {
        CHECK(_optional, "Bad optional access");
        return &_optional.value()._value;
    }

private:
    std::optional<Stub> _optional;
};

}

#endif