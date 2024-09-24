#pragma once

#include <functional>

#include "core/inf/variable.h"
#include "core/types/implements.h"

namespace ark {

template<typename T, typename U = T> class VariableOP1 final : public Variable<T>, Implements<VariableOP1<T>, Variable<T>> {
private:
    typedef std::function<T(U)> OPFunc;

public:
    VariableOP1(OPFunc func, sp<Variable<U>> arg)
        : _func(std::move(func)), _arg(std::move(arg)) {
    }

    T val() override {
        return _func(_arg->val());
    }

    bool update(uint64_t timestamp) override {
        return _arg->update(timestamp);
    }

private:
    OPFunc _func;
    sp<Variable<U>> _arg;
};

}
