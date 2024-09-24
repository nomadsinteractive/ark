#pragma once

#include <type_traits>

#include "core/inf/variable.h"
#include "core/types/implements.h"

namespace ark {

namespace {

template<typename T> auto _op_type_sfinae(const T& p, decltype(p->val())*) -> decltype(p->val());
template<typename T> T _op_type_sfinae(const T& /*p*/, ...);

}

template<typename T, typename U, typename OP2, typename LTYPE = decltype(_op_type_sfinae<T>(std::declval<T>(), nullptr)), typename RTYPE = decltype(_op_type_sfinae<U>(std::declval<U>(), nullptr)),
         typename OPType = std::invoke_result_t<OP2, LTYPE, RTYPE>> class VariableOP2 final : public Variable<OPType>, Implements<VariableOP2<T, U, OP2, LTYPE, RTYPE, OPType>, Variable<OPType>> {
public:
    VariableOP2(T p1, U p2)
        : _lv(std::move(p1)), _rv(std::move(p2)) {
    }

    OPType val() override {
        auto lv = _val_sfinae<T, LTYPE>(_lv, nullptr);
        auto rv = _val_sfinae<U, RTYPE>(_rv, nullptr);
        return _op2(std::move(lv), std::move(rv));
    }

    bool update(uint64_t timestamp) override {
        const bool d1 = _update_sfinae(_lv, timestamp, nullptr);
        const bool d2 = _update_sfinae(_rv, timestamp, nullptr);
        return d1 || d2;
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

private:
    T _lv;
    U _rv;
    OP2 _op2;
};

}
