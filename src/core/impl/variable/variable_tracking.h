#pragma once

#include "core/inf/variable.h"
#include "core/util/math.h"

namespace ark {

template<typename T> class VariableTracking final : public Variable<T> {
public:
    VariableTracking(sp<Variable<T>> target, sp<Numeric> t, const T s0, const float speed, const float snapDistance2)
        :  _target(std::move(target)), _t(std::move(t)), _value(s0), _speed(speed), _snap_distance2(snapDistance2) {
    }

    bool update(uint64_t timestamp) override {
        const float t1 = _t->val();
        const bool dirty = _target->update(timestamp);
        const T targetValue = _target->val();
        const T vec = targetValue - _value;
        const float distance2 = Math::hypot2(vec);
        if(distance2 < _snap_distance2) {
            _value = targetValue;
            _t0 = t1;
            return dirty;
        }
        float distance;
        if constexpr(std::is_same_v<T, float>)
            distance = std::abs(vec);
        else
            distance = Math::sqrt(distance2);
        const T n = vec / distance;
        _value += n * std::min((t1 - _t0) * _speed,  distance);
        _t0 = t1;
        return true;
    }

    T val() override {
        return _value;
    }

private:
    sp<Variable<T>> _target;
    sp<Numeric> _t;
    float _speed;

    T _value;
    float _t0;
    float _snap_distance2;
};

}