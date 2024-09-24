#pragma once

#include "core/forwarding.h"
#include "core/inf/variable.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class Integral final : public Variable<T>, Implements<Integral<T>, Variable<T>> {
public:
    Integral(sp<Variable<T>> v, sp<Numeric> t)
        : _v(std::move(v)), _t(std::move(t)), _s0(0), _v0(_v->val()), _t0(_t->val()) {
    }

    T val() override {
        T v = _v->val();
        float t = _t->val();
        _s0 += ((v + _v0) / 2) * (t - _t0);
        _v0 = v;
        _t0 = t;
        return _s0;
    }

    bool update(uint64_t timestamp) override {
        return _t->update(timestamp);
    }

private:
    sp<Variable<T>> _v;
    sp<Numeric> _t;

    T _s0;
    T _v0;
    float _t0;
};

template<typename T> class IntegralS2 final : public Variable<T>, Implements<IntegralS2<T>, Variable<T>> {
public:
    IntegralS2(sp<Variable<T>> a, sp<Numeric> t, const T& s0, const T& s1)
        : _a(std::move(a)), _t(std::move(t)), _s0(s0), _s1(s1), _a1(0), _t0(_t->val()), _t1(_t0) {
    }

    T val() override {
        float t = _t->val();
        T s2 = _s1 * 2 - _s0 + (t - _t1) * (_t1 - _t0) * _a1;
        _a1 = _a->val();
        _s0 = _s1;
        _s1 = s2;
        _t0 = _t1;
        _t1 = t;
        return s2;
    }

    bool update(uint64_t timestamp) override {
        return _t->update(timestamp);
    }

private:
    sp<Variable<T>> _a;
    sp<Numeric> _t;

    T _s0;
    T _s1;
    T _a1;
    float _t0;
    float _t1;
};

}
