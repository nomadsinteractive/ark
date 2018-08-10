#ifndef ARK_CORE_IMPL_VARIABLE_VARIABLE_OP2_H_
#define ARK_CORE_IMPL_VARIABLE_VARIABLE_OP2_H_

#include <type_traits>

namespace ark {

template<typename P, typename OP2, typename LVType, typename RVType, typename OPType = typename std::result_of<OP2(P, P)>::type> class VariableOP2 : public Variable<OPType> {
public:
    VariableOP2(const LVType& p1, const RVType& p2)
        : _p1(p1), _p2(p2) {
    }

    virtual OPType val() override {
        return _op2(_val_sfinae(_p1, nullptr), _val_sfinae(_p2, nullptr));
    }

private:
    template<typename T> static P _val_sfinae(const T& p, decltype(p->val())* /*args*/) {
        return static_cast<P>(p->val());
    }

    template<typename T> static P _val_sfinae(const T& p, ...) {
        return static_cast<P>(p);
    }

private:
    LVType _p1;
    RVType _p2;
    OP2 _op2;
};

}

#endif
