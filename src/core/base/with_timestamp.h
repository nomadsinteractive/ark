#ifndef ARK_CORE_BASE_WITH_TIMESTAMP_H_
#define ARK_CORE_BASE_WITH_TIMESTAMP_H_

#include "core/base/timestamp.h"

namespace ark {

template<typename T> class WithTimestamp {
public:
    WithTimestamp() = default;
    WithTimestamp(const T& value)
        : _value(value) {
    }
    DISALLOW_COPY_AND_ASSIGN(WithTimestamp);

    const T* operator ->() const {
        return &_value;
    }

    operator const T&() const {
        return _value;
    }

    bool update(uint64_t timestamp) const {
        return _timestamp.update(timestamp);
    }

    void reset(const T& other) {
        _value = other;
        _timestamp.markDirty();
    }

private:
    T _value;
    Timestamp _timestamp;
};

}
#endif
