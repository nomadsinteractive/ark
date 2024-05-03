#ifndef ARK_GRAPHICS_IMPL_VEC_VEC_SUBSCRIBED_H_
#define ARK_GRAPHICS_IMPL_VEC_VEC_SUBSCRIBED_H_

#include <array>

#include "core/inf/variable.h"
#include "core/base/wrapper.h"

#include "graphics/forwarding.h"

namespace ark {

template<typename T, typename U> class VecSubscribed : public Wrapper<Variable<U>>, public Variable<T> {
public:
    typedef std::array<size_t, sizeof(T) / sizeof(float)> IndexType;

    VecSubscribed(sp<Variable<U>> delegate, IndexType indices)
        : Wrapper<Variable<U>>(std::move(delegate)), _indices(std::move(indices)) {
    }

    virtual bool update(uint64_t timestamp) override {
        return this->_wrapped->update(timestamp);
    }

    virtual T val() override {
        T r;
        U v = this->_wrapped->val();
        for(size_t i = 0; i < _indices.size(); ++i)
            r[i] = v[_indices[i]];
        return r;
    }

private:
    IndexType  _indices;

};

}

#endif
