#ifndef ARK_CORE_IMPL_VARIABLE_VARIABLE_DYED_H_
#define ARK_CORE_IMPL_VARIABLE_VARIABLE_DYED_H_

#include "core/forwarding.h"
#include "core/base/wrapper.h"
#include "core/base/string.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"
#include "core/types/safe_var.h"

namespace ark {

template<typename T> class VariableDyed : public Wrapper<Variable<T>>, public Variable<T> {
public:
    VariableDyed(sp<Variable<T>> delegate, sp<Boolean> condition, String message)
        :  Wrapper<Variable<T>>(std::move(delegate)), _condition(std::move(condition), true), _message(std::move(message)) {
    }

    virtual bool update(uint64_t timestamp) override {
        _condition.update(timestamp);
        return this->_wrapped->update(timestamp);
    }

    virtual T val() override {
        TRACE(_condition.val(), _message.c_str());
        return this->_wrapped->val();
    }

private:
    SafeVar<Boolean> _condition;
    String _message;

};

}

#endif
