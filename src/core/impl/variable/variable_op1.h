#pragma once

#include <functional>

#include "core/inf/holder.h"
#include "core/inf/variable.h"
#include "core/types/implements.h"
#include "core/util/holder_util.h"

namespace ark {

template<typename T, typename U = T> class VariableOP1 : public Variable<T>, public Holder, Implements<VariableOP1<T>, Variable<T>, Holder> {
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

    void traverse(const Visitor& visitor) override {
        HolderUtil::visit(_arg, visitor);
    }

private:
    OPFunc _func;
    sp<Variable<U>> _arg;
};

}
