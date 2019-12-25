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
        : _v(std::move(v)), _t(std::move(t)), _s(), _last_v(_v->val()), _last_t(_t->val()) {
    }

    virtual T val() override {
        T v = _v->val();
        float t = _t->val();
        _s += ((v + _last_v) / 2) * (t - _last_t);
        _last_v = v;
        _last_t = t;
        return _s;
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

    T _s;
    T _last_v;
    float _last_t;
};

}

#endif
