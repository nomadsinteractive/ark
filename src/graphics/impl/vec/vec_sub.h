#ifndef ARK_GRAPHICS_IMPL_VEC_VEC_SUB_H_
#define ARK_GRAPHICS_IMPL_VEC_VEC_SUB_H_

#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

template<typename T> class VecSub : public Variable<T> {
public:
    VecSub(const sp<Variable<T>>& lvalue, const sp<Variable<T>>& rvalue)
        : _lvalue(lvalue), _rvalue(rvalue) {
    }

    virtual T val() override {
        return _lvalue->val() - _rvalue->val();
    }

private:
    sp<Variable<T>> _lvalue;
    sp<Variable<T>> _rvalue;
};

}

#endif