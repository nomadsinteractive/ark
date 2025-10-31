#pragma once

#include <type_traits>

#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class VariableTernary final : public Variable<T> {
public:
    VariableTernary(sp<Boolean> condition, sp<Variable<T>> a, sp<Variable<T>> b)
        : _condition(std::move(condition)), _a(std::move(a)), _b(std::move(b)) {
        ASSERT(_condition && _a && _b);
    }

    T val() override {
        return _condition->val() ? _a->val() : _b->val();
    }

    bool update(uint32_t tick) override {
        const bool dirty = _condition->update(tick);
        return (_condition->val() ? _a->update(tick) : _b->update(tick)) || dirty;
    }

private:
    sp<Boolean> _condition;
    sp<Variable<T>> _a;
    sp<Variable<T>> _b;
};

}
