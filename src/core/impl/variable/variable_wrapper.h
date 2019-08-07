#ifndef ARK_CORE_BASE_VARIABLE_WRAPPER_H_
#define ARK_CORE_BASE_VARIABLE_WRAPPER_H_

#include "core/forwarding.h"
#include "core/ark.h"
#include "core/base/delegate.h"
#include "core/inf/variable.h"
#include "core/types/class.h"
#include "core/types/null.h"

#include "app/base/application_context.h"

namespace ark {

template<typename T> class VariableWrapper final : public Variable<T>, public Delegate<Variable<T>>, Implements<VariableWrapper<T>, Variable<T>, Delegate<Variable<T>>> {
public:
    VariableWrapper(const sp<Variable<T>>& delegate) noexcept
        : Delegate<Variable<T>>(Null::toSafe(delegate)), _variable_impl(nullptr) {
    }
    VariableWrapper(T value) noexcept
        : Delegate<Variable<T>>(sp<typename Variable<T>::Impl>::make(value)), _variable_impl(static_cast<typename Variable<T>::Impl*>(this->_delegate.get())) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(VariableWrapper);

    virtual T val() override {
        return this->_delegate->val();
    }

    void set(T value) {
        if(_variable_impl)
            _variable_impl->set(value);
        else {
            deferedUnref();
            _variable_impl = new typename Variable<T>::Impl(value);
            this->_delegate = sp<Variable<T>>::adopt(_variable_impl);
        }
    }

    void set(const sp<Variable<T>>& delegate) {
        deferedUnref();
        DCHECK(delegate.get() != this, "Recursive delegate being set");
        this->_delegate = Null::toSafe(delegate);
    }

    T fix() {
        T val = this->_delegate->val();
        set(val);
        return val;
    }

private:
    void deferedUnref() {
        _variable_impl = nullptr;
        if(this->_delegate)
            Ark::instance().applicationContext()->deferUnref(std::move(this->_delegate));
    }

private:
    typename Variable<T>::Impl* _variable_impl;
};

}

#endif
