#pragma once

#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"
#include "core/util/math.h"
#include "core/util/updatable_util.h"

namespace ark {

template<typename T> class SecondOrderDynamics final : public Variable<T>, Implements<SecondOrderDynamics<T>, Variable<T>> {
public:
    /**
     * @param x x
     * @param s0 start: s0
     * @param t time: y = sod(f(t))
     * @param f nature frequence(HZ): the damping frequence that y follows x
     * @param z zeta: damping coefficient, how the system settles at the target.
     *          zeta(0)      undamped, y would be always damping around x.
     *          zeta(0 - 1)  underdamped, y would be underdamping around x and then following x eventually.
     *          zeta(1)      critical damping, there would be no vibrate motion.
     *          zeta(1 - )   nodamp, same as critical damping. the greater z is, the slower motion is.
     * @param r initial response:
     *          r(0)         the system would take time to begin to acceleration.
     *          r(0 - 1)     the system would react immediately to x.
     *          r(1 - )      the system would overshoot x depends on the magnitude of r.
     *          r( - 0)      the system would anticipate the motion(moving backward and then towards).
     */
    SecondOrderDynamics(sp<Variable<T>> x, const T& s0, sp<Numeric> t, const float f, const float z = 1.0f, const float r = 0)
         : _x(std::move(x)), _t(std::move(t)), _w(Math::TAU * f), _z(z), _d(_w * std::sqrt(std::abs(z * z - 1))), _k1(z / (Math::PI * f)), _k2(1 / _w / _w), _k3(r * z / _w),
           _last_t(_t->val()), _last_x(_x->val()), _y(s0), _dy(0) {
    }

    T val() override {
        const float t = _t->val();
        const float dt = t - _last_t;
        if(dt > 0) {
            const T x = _x->val();
            const T dx = (x - _last_x) / dt;
            _last_x = x;
            _last_t = t;
            float k1Stable;
            float k2Stable;
            if(_w * dt < _z) {
                k1Stable = _k1;
                k2Stable = std::max(_k2, std::max(dt * dt / 2 + dt * _k1 / 2, dt * _k1));
            } else {
                const float t1 = std::exp(-_z * _w * dt);
                const float alpha = 2 * t1 * (_z <= 1 ? std::cos(dt * _d) : std::cosh(dt * _d));
                const float beta = t1 * t1;
                const float t2 = dt / (1 + beta - alpha);
                k1Stable = (1 - beta) * t2;
                k2Stable = dt * t2;
            }
            _y += dt * _dy;
            _dy += dt * (x + _k3 * dx - _y - k1Stable * _dy) / k2Stable;
        }
        return _y;
    }

    bool update(const uint64_t timestamp) override {
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
