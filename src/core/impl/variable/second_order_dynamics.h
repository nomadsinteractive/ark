#ifndef ARK_CORE_IMPL_SECOND_ORDER_DYNAMICS_H_
#define ARK_CORE_IMPL_SECOND_ORDER_DYNAMICS_H_

#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"
#include "core/util/math.h"
#include "core/util/updatable_util.h"

namespace ark {

template<typename T> class SecondOrderDynamics : public Variable<T>, Implements<SecondOrderDynamics<T>, Variable<T>> {
public:
    SecondOrderDynamics(sp<Variable<T>> x, sp<Numeric> t, float f, float z, float r)
         : _x(std::move(x)), _t(std::move(t)), _w(2 * Math::PI * f), _z(z), _d(_w * std::sqrt(std::abs(z * z - 1))), _k1(z / (Math::PI * f)), _k2(1 / _w / _w), _k3(r * z / _w),
           _last_t(_t->val()), _last_x(_x->val()), _y(_last_x), _dy(0) {
    }

    virtual T val() override {
        float t = _t->val();
        float dt = t - _last_t;
        const T x = _x->val();
        const T dx = (x - _last_x) / dt;
        _last_x = x;
        _last_t = t;
        float k1Stable;
        float k2Stable;
        if(_w * dt < _z || true) {
            k1Stable = _k1;
            k2Stable = std::max(_k2, std::max(dt * dt / 2 + dt * _k1 / 2, dt * _k1));
        } else {
            float t1 = std::exp(-_z * _w * dt);
            float alpha = 2 * t1 * (_z <= 1 ? std::cos(dt * _d) : std::cosh(dt * _d));
            float beta = t1 * t1;
            float t2 = dt / (1 + beta - alpha);
            k1Stable = (1 - beta) * t2;
            k2Stable = dt * t2;
        }
        _y += dt * _dy;
        _dy += dt * (x + _k3 * dx - _y - k1Stable * _dy) / k2Stable;
        return _y;
    }

    virtual bool update(uint64_t timestamp) override {
        return UpdatableUtil::update(timestamp, _x, _t);
    }

private:
    sp<Variable<T>> _x;
    sp<Numeric> _t;
    float _w, _z, _d;
    float _k1, _k2, _k3;
    float _last_t;
    T _last_x;
    T _y;
    T _dy;
};

}

#endif
