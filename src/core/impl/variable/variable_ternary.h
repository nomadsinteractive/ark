#pragma once

#include <type_traits>

#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class VariableTernary final : public Variable<T> {
public:
    VariableTernary(sp<Boolean> condition, sp<Variable<T>> a, sp<Variable<T>> b)
        : _condition(std::move(condition)), _a(std::move(a)), _b(std::move(b)) {
    }

    virtual T val() override {
        return _condition->val() ? _a->val() : _b->val();
    }

    virtual bool update(uint64_t timestamp) override {
        bool dirty = _condition->update(timestamp);
        return (_condition->val() ? _a->update(timestamp) : _b->update(timestamp)) || dirty;
    }

private:
    sp<Boolean> _condition;
    sp<Variable<T>> _a;
    sp<Variable<T>> _b;
};

}
