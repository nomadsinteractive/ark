#pragma once

#include "core/forwarding.h"
#include "core/base/wrapper.h"
#include "core/base/string.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"
#include "core/types/optional_var.h"

namespace ark {

template<typename T> class VariableDyed final : public Wrapper<Variable<T>>, public Variable<T> {
public:
    VariableDyed(sp<Variable<T>> delegate, sp<Boolean> condition, String message)
        :  Wrapper<Variable<T>>(std::move(delegate)), _condition(std::move(condition), true), _message(std::move(message)) {
    }

    bool update(uint64_t timestamp) override {
        _condition.update(timestamp);
        return this->_wrapped->update(timestamp);
    }

    T val() override {
        TRACE(_condition.val(), _message.c_str());
        return this->_wrapped->val();
    }

private:
    OptionalVar<Boolean> _condition;
    String _message;
};

}
