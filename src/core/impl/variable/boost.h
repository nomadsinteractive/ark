#ifndef ARK_CORE_IMPL_VARIABLE_BOOST_H_
#define ARK_CORE_IMPL_VARIABLE_BOOST_H_

#include "core/forwarding.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class Boost : public Variable<T> {
public:
    Boost(const sp<Variable<T>>& a, const T& v0, const sp<Numeric>& cd, const sp<Numeric>& t)
        : _v(v0), _a(a), _cd(cd), _t(t), _last_t(_t->val()) {
        DWARN(_cd->val() >= 0, "The drag coefficient factor should be greater than 0, got %.2f", _cd->val());
    }

    virtual T val() override {
        float t = _t->val();
        float dt = (t - _last_t);
        T v = _v;
        T a = _a->val() - getRC_sfinae<T>(v, _cd->val(), nullptr);
        _v = v + a * dt;
        _last_t = t;
        return _v;
    }

private:

    template<typename U> U getRC_sfinae(const U& v, float cd, decltype(v.length())*) const {
        auto length = v.length();
        return v * length * cd;
    }

    template<typename U> U getRC_sfinae(const U& v, float cd, ...) const {
        auto length = std::abs(v);
        return v * length * cd;
    }

private:
    T _v;
    sp<Variable<T>> _a;
    sp<Numeric> _cd;
    sp<Numeric> _t;
    float _last_t;
};

}

#endif
