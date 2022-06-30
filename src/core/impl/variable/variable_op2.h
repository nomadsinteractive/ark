#ifndef ARK_CORE_IMPL_VARIABLE_VARIABLE_OP2_H_
#define ARK_CORE_IMPL_VARIABLE_VARIABLE_OP2_H_

#include <type_traits>

#include "core/inf/holder.h"
#include "core/inf/variable.h"
#include "core/types/implements.h"
#include "core/util/holder_util.h"

namespace ark {

template<typename T> auto _op_type_sfinae(const T& p, decltype(p->val())*) -> decltype(p->val());
template<typename T> T _op_type_sfinae(const T& /*p*/, ...);

template<typename T, typename U, typename OP2, typename LTYPE = decltype(_op_type_sfinae<T>(std::declval<T>(), nullptr)), typename RTYPE = decltype(_op_type_sfinae<U>(std::declval<U>(), nullptr)),
         typename OPType = std::invoke_result_t<OP2, LTYPE, RTYPE>> class VariableOP2 : public Variable<OPType>, public Holder, Implements<VariableOP2<T, U, OP2, LTYPE, RTYPE, OPType>, Variable<OPType>, Holder> {
public:
    VariableOP2(const T& p1, const U& p2)
        : _lv(p1), _rv(p2) {
    }

    virtual OPType val() override {
        return _op2(_val_sfinae<T, LTYPE>(_lv, nullptr), _val_sfinae<U, RTYPE>(_rv, nullptr));
    }

    virtual bool update(uint64_t timestamp) override {
        bool d1 = _update_sfinae(_lv, timestamp, nullptr);
        bool d2 = _update_sfinae(_rv, timestamp, nullptr);
        return d1 || d2;
    }

    virtual void traverse(const Visitor& visitor) override {
        _visit_sfinae(_lv, visitor, nullptr);
        _visit_sfinae(_rv, visitor, nullptr);
    }

private:
    template<typename V> static bool _update_sfinae(const V& p, uint64_t timestamp, decltype(p->update(0))* /*args*/) {
        return p->update(timestamp);
    }

    template<typename V> static bool _update_sfinae(const V& /*p*/, uint64_t /*timestamp*/, ...) {
        return false;
    }

    template<typename V, typename W> static W _val_sfinae(const V& p, decltype(p->val())* /*args*/) {
        return static_cast<W>(p->val());
    }

    template<typename V, typename W> static W _val_sfinae(const V& p, ...) {
        return static_cast<W>(p);
    }

    template<typename V> static void _visit_sfinae(const V& p, const Visitor& visitor, decltype(p->val())* /*args*/) {
        HolderUtil::visit(p, visitor);
    }

    template<typename V> static void _visit_sfinae(const V& /*p*/, const Visitor& /*visitor*/, ...) {
    }

private:
    T _lv;
    U _rv;
    OP2 _op2;

};

}

#endif
