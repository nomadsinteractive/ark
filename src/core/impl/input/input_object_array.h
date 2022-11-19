#ifndef ARK_CORE_IMPL_INPUT_INPUT_OBJECT_ARRAY_H_
#define ARK_CORE_IMPL_INPUT_INPUT_OBJECT_ARRAY_H_

#include <vector>

#include "core/inf/input.h"


namespace ark {

template<typename T> class InputObjectArray : public Input {
public:
    InputObjectArray(std::vector<T> vector)
        : _vector(std::move(vector)) {
    }

    virtual void flat(void* buf) override {
        memcpy(buf, _vector.data(), _vector.size() * sizeof(T));
    }

    virtual uint32_t size() override {
        return static_cast<uint32_t>(_vector.size() * sizeof(T));
    }

    virtual bool update(uint64_t /*timestamp*/) override {
        return false;
    }

private:
    std::vector<T> _vector;

};

}

#endif
