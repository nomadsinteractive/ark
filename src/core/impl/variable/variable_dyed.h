#pragma once

#include "core/forwarding.h"
#include "core/base/wrapper.h"
#include "core/base/string.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"
#include "core/types/safe_var.h"
#include "core/util/strings.h"

namespace ark {

template<typename T> class VariableDyed : public Wrapper<Variable<T>>, public Variable<T> {
public:
    VariableDyed(sp<Variable<T>> delegate, sp<Boolean> condition, String message)
        :  Wrapper<Variable<T>>(std::move(delegate)), _condition(std::move(condition), true), _message(std::move(message)) {
    }

    bool update(uint64_t timestamp) override {
        _condition.update(timestamp);
        return this->_wrapped->update(timestamp);
    }

    T val() override {
        T v = this->_wrapped->val();
        TRACE(_condition.val(), Strings::sprintf("%s: %s", _message.c_str(), Strings::toString(v).c_str()).c_str());
        return v;
    }

private:
    SafeVar<Boolean> _condition;
    String _message;

};

}
