#ifndef ARK_CORE_IMPL_VARIABLE_VARIABLE_OP1_H_
#define ARK_CORE_IMPL_VARIABLE_VARIABLE_OP1_H_

#include <functional>

#include "core/base/timestamp.h"
#include "core/inf/holder.h"
#include "core/inf/variable.h"
#include "core/types/implements.h"
#include "core/util/holder_util.h"
#include "core/util/variable_util.h"

namespace ark {

template<typename T, typename U = T> class VariableOP1 : public Variable<T>, public Holder, Implements<VariableOP1<T>, Variable<T>, Holder> {
private:
    typedef std::function<T(U)> OPFunc;

public:
    VariableOP1(OPFunc func, const sp<Variable<U>>& arg)
        : _func(std::move(func)), _arg(arg), _val(_func(_arg->val())) {
    }

    virtual T val() override {
        return _val;
    }

    virtual bool update(uint64_t timestamp) override {
        if(!VariableUtil::update(timestamp, _arg, _timestamp))
            return false;

        _val = _func(_arg->val());
        return true;
    }

    virtual void traverse(const Visitor& visitor) override {
        HolderUtil::visit(_arg, visitor);
    }

private:
    OPFunc _func;
    sp<Variable<U>> _arg;

    T _val;
    Timestamp _timestamp;
};

}

#endif
