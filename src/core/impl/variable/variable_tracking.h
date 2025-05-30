#pragma once

#include "core/base/future.h"
#include "core/inf/variable.h"
#include "core/util/math.h"

namespace ark {

template<typename T> class VariableTracking final : public Variable<T> {
public:
    VariableTracking(sp<Variable<T>> target, sp<Numeric> t, const T s0, const float speed, const float distance, sp<Future> future = nullptr)
        :  _target(std::move(target)), _t(std::move(t)), _value(s0), _speed(speed), _distance(distance), _future(std::move(future)), _t0(0) {
    }

    bool update(uint64_t timestamp) override {
        const bool dirty = _target->update(timestamp);
        if(_future && _future->isDoneOrCanceled())
            return dirty;

        const float t1 = _t->val();
        const T targetValue = _target->val();
        const T vec = targetValue - _value;
        const float distance2 = Math::hypot2(vec);
        float distance;
        if constexpr(std::is_same_v<T, float>)
            distance = std::abs(vec);
        else
            distance = Math::sqrt(distance2);
        if(distance > _distance) {
            const T n = vec / distance;
            _value += n * std::min((t1 - _t0) * _speed, distance);
            _t0 = t1;
        }
        else if(_future)
            _future->notify();
        return true;
    }

    T val() override {
        return _value;
    }

private:
    sp<Variable<T>> _target;
    sp<Numeric> _t;
    T _value;
    float _speed;
    float _distance;
    sp<Future> _future;
    float _t0;
};

}