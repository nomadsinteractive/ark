#ifndef ARK_CORE_IMPL_VARIABLE_INTEGRAL_H_
#define ARK_CORE_IMPL_VARIABLE_INTEGRAL_H_

#include "core/forwarding.h"
#include "core/inf/holder.h"
#include "core/inf/variable.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"
#include "core/util/holder_util.h"

namespace ark {

template<typename T> class Integral : public Variable<T>, public Holder, Implements<Integral<T>, Variable<T>, Holder> {
public:
    Integral(sp<Variable<T>> v, sp<Numeric> t)
        : _v(std::move(v)), _t(std::move(t)), _s0(0), _v0(_v->val()), _t0(_t->val()) {
    }

    virtual T val() override {
        T v = _v->val();
        float t = _t->val();
        _s0 += ((v + _v0) / 2) * (t - _t0);
        _v0 = v;
        _t0 = t;
        return _s0;
    }

    virtual bool update(uint64_t timestamp) override {
        return _t->update(timestamp);
    }

    virtual void traverse(const Visitor& visitor) override {
        HolderUtil::visit(_v, visitor);
        HolderUtil::visit(_t, visitor);
    }

private:
    sp<Variable<T>> _v;
    sp<Numeric> _t;

    T _s0;
    T _v0;
    float _t0;
};

template<typename T> class IntegralS2 : public Variable<T>, public Holder, Implements<IntegralS2<T>, Variable<T>, Holder> {
public:
    IntegralS2(sp<Variable<T>> a, sp<Numeric> t, const T& s0, const T& s1 = s0)
        : _a(std::move(a)), _t(std::move(t)), _s0(s0), _s1(s1), _t0(_t->val()), _t1(_t0), _a1(0) {
    }

    virtual T val() override {
        float t = _t->val();
        T s2 = _s1 * 2 - _s0 + (t - _t1) * (_t1 - _t0) * _a1;
        _a1 = _a->val();
        _s0 = _s1;
        _s1 = s2;
        _t0 = _t1;
        _t1 = t;
        return s2;
    }

    virtual bool update(uint64_t timestamp) override {
        return _t->update(timestamp);
    }

    virtual void traverse(const Visitor& visitor) override {
        HolderUtil::visit(_a, visitor);
        HolderUtil::visit(_t, visitor);
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

#endif
