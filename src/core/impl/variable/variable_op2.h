#ifndef ARK_CORE_IMPL_VARIABLE_VARIABLE_OP2_H_
#define ARK_CORE_IMPL_VARIABLE_VARIABLE_OP2_H_

#include <type_traits>

#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"


namespace ark {

template<typename P, typename OP2, typename T = std::result_of<OP2(P, P)>::type> class VariableOP2 : public Variable<T> {
public:
    typedef sp<Variable<P>> VType;

    VariableOP2(const VType& p1, const VType& p2)
        : _p1(p1), _p2(p2) {
    }

    virtual T val() override {
        return _op2(_p1->val(), _p2->val());
    }

private:
    VType _p1;
    VType _p2;
    OP2 _op2;
};

}

#endif
