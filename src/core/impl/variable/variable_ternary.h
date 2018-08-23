#ifndef ARK_CORE_IMPL_VARIABLE_VARIABLE_TERNARY_H_
#define ARK_CORE_IMPL_VARIABLE_VARIABLE_TERNARY_H_

#include <type_traits>

#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class VariableTernary : public Variable<T> {
public:
    VariableTernary(const sp<Boolean>& condition, const sp<Variable<T>>& a, const sp<Variable<T>>& b)
        : _condition(condition), _a(a), _b(b) {
    }

    virtual T val() override {
        return _condition->val() ? _a->val() : _b->val();
    }

private:
    sp<Boolean> _condition;
    sp<Variable<T>> _a;
    sp<Variable<T>> _b;
};

}

#endif
