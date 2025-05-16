#pragma once

#include "core/base/timestamp.h"

namespace ark {

template<typename T> class WithTimestamp {
public:
    WithTimestamp() = default;
    WithTimestamp(T value)
        : _value(std::move(value)) {
    }
    DISALLOW_COPY_AND_ASSIGN(WithTimestamp);

    const T* operator ->() const {
        return &_value;
    }

    operator const T&() const {
        return _value;
    }

    T value() const {
        return _value;
    }

    bool update(const uint64_t timestamp) const {
        return _timestamp.update(timestamp);
    }

    void reset(T other) {
        _value = std::move(other);
        _timestamp.markDirty();
    }

private:
    T _value;
    Timestamp _timestamp;
};

}
