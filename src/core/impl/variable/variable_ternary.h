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

    bool update(const uint64_t timestamp) override {
        const bool dirty = _condition->update(timestamp);
        return (_condition->val() ? _a->update(timestamp) : _b->update(timestamp)) || dirty;
    }

private:
    sp<Boolean> _condition;
    sp<Variable<T>> _a;
    sp<Variable<T>> _b;
};

}
