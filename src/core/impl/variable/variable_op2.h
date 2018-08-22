#ifndef ARK_CORE_IMPL_VARIABLE_VARIABLE_OP2_H_
#define ARK_CORE_IMPL_VARIABLE_VARIABLE_OP2_H_

#include <type_traits>

namespace ark {

template<typename T, typename P, typename OP2, typename LVType, typename RVType, typename OPType = typename std::result_of<OP2(T, P)>::type> class VariableOP2 : public Variable<OPType> {
public:
    VariableOP2(const LVType& p1, const RVType& p2)
        : _p1(p1), _p2(p2) {
    }

    virtual OPType val() override {
        return _op2(_val_sfinae<LVType, T>(_p1, nullptr), _val_sfinae<RVType, P>(_p2, nullptr));
    }

private:
    template<typename U, typename V> static V _val_sfinae(const U& p, decltype(p->val())* /*args*/) {
        return static_cast<V>(p->val());
    }

    template<typename U, typename V> static V _val_sfinae(const U& p, ...) {
        return static_cast<V>(p);
    }

private:
    LVType _p1;
    RVType _p2;
    OP2 _op2;
};

}

#endif
