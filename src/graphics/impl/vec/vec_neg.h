#ifndef ARK_GRAPHICS_IMPL_VEC_VEC_NEG_H_
#define ARK_GRAPHICS_IMPL_VEC_VEC_NEG_H_

#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

template<typename T> class VecNeg : public Variable<T> {
public:
    VecNeg(const sp<Variable<T>>& delegate)
        : _delegate(delegate) {
    }

    virtual T val() override {
        return -(_delegate->val());
    }

private:
    sp<Variable<T>> _delegate;
};

}

#endif
