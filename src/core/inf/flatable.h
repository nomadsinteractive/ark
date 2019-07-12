#ifndef ARK_CORE_INF_FLATABLE_H_
#define ARK_CORE_INF_FLATABLE_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/array.h"

namespace ark {

class ARK_API Flatable {
public:
    virtual ~Flatable() = default;

    virtual void flat(void* buf) = 0;
    virtual uint32_t size() = 0;

    template<typename T> class Array;
};

template<typename T> class Flatable::Array : public Flatable {
public:
    Array(array<T> values)
        : _values(std::move(values)) {
    }

    virtual void flat(void* buf) override {
        memcpy(buf, _values->buf(), size());
    }

    virtual uint32_t size() override {
        return _values->size();
    }

private:
    array<T> _values;
};

}

#endif
