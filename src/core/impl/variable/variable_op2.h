#ifndef ARK_CORE_IMPL_VARIABLE_VARIABLE_OP2_H_
#define ARK_CORE_IMPL_VARIABLE_VARIABLE_OP2_H_

#include <type_traits>

#include "core/inf/holder.h"
#include "core/inf/variable.h"
#include "core/types/implements.h"
#include "core/util/holder_util.h"

namespace ark {

template<typename T, typename P, typename OP2, typename LVType, typename RVType, typename OPType = typename std::result_of<OP2(T, P)>::type> class VariableOP2 : public Variable<OPType>, public Holder, Implements<VariableOP2<T, P, OP2, LVType, RVType, OPType>, Variable<OPType>, Holder> {
public:
    VariableOP2(const LVType& p1, const RVType& p2)
        : _p1(p1), _p2(p2) {
    }

    virtual OPType val() override {
        return _op2(_val_sfinae<LVType, T>(_p1, nullptr), _val_sfinae<RVType, P>(_p2, nullptr));
    }

    virtual void traverse(const Visitor& visitor) override {
        _visit_sfinae(_p1, visitor, nullptr);
        _visit_sfinae(_p2, visitor, nullptr);
    }

private:
    template<typename U, typename V> static V _val_sfinae(const U& p, decltype(p->val())* /*args*/) {
        return static_cast<V>(p->val());
    }

    template<typename U, typename V> static V _val_sfinae(const U& p, ...) {
        return static_cast<V>(p);
    }

    template<typename U> static void _visit_sfinae(const U& p, const Visitor& visitor, decltype(p->val())* /*args*/) {
        HolderUtil::visit(p, visitor);
    }

    template<typename U> static void _visit_sfinae(const U& /*p*/, const Visitor& /*visitor*/, ...) {
    }

private:
    LVType _p1;
    RVType _p2;
    OP2 _op2;

};

}

#endif
