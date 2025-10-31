#pragma once

#include "core/forwarding.h"
#include "core/inf/variable.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"
#include "core/util/updatable_util.h"

namespace ark {

template<typename T, typename U> class Lerp final : public Variable<T>, Implements<Lerp<T, U>, Variable<T>> {
public:
    Lerp(sp<Variable<T>> a, sp<Variable<T>> b, sp<Variable<U>> v)
        : _a(std::move(a)), _b(std::move(b)), _v(std::move(v)), _a_freezed(_a->val()), _delta_freezed(_b->val() - _a_freezed), _val(_a_freezed + _delta_freezed * _v->val()) {
    }

    T val() override {
        return _val;
    }

    bool update(uint32_t tick) override {
        bool dirty = UpdatableUtil::update(tick, _a, _b);
        if(dirty) {
            _a_freezed = _a->val();
            _delta_freezed = _b->val() - _a_freezed;
        }
        if(_v->update(tick) || dirty) {
            _val = _a_freezed + _delta_freezed * _v->val();
            return true;
        }
        return false;
    }

private:
    sp<Variable<T>> _a;
    sp<Variable<T>> _b;
    sp<Variable<U>> _v;

    T _a_freezed;
    T _delta_freezed;
    T _val;
};

}
