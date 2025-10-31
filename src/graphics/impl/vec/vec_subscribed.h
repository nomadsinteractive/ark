#pragma once

#include <array>

#include "core/inf/variable.h"
#include "core/base/wrapper.h"

namespace ark {

template<typename T, typename U> class VecSubscribed final : public Wrapper<Variable<U>>, public Variable<T> {
private:
    constexpr static size_t DIMENSION = sizeof(T) / sizeof(float);

public:
    typedef std::conditional_t<DIMENSION == 1, uint8_t, std::array<uint8_t, DIMENSION>> IndexType;

    VecSubscribed(sp<Variable<U>> delegate, IndexType indices)
        : Wrapper<Variable<U>>(std::move(delegate)), _indices(indices) {
    }

    bool update(uint32_t tick) override {
        return this->_wrapped->update(tick);
    }

    T val() override {
        T r;
        U v = this->_wrapped->val();
        if constexpr(DIMENSION == 1)
            r = v[_indices];
        else
            for(size_t i = 0; i < _indices.size(); ++i)
                r[i] = v[_indices[i]];
        return r;
    }

private:
    IndexType  _indices;
};

}
