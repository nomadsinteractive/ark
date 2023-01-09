#ifndef ARK_CORE_BASE_VARIABLE_WRAPPER_H_
#define ARK_CORE_BASE_VARIABLE_WRAPPER_H_

#include "core/forwarding.h"
#include "core/ark.h"
#include "core/base/wrapper.h"
#include "core/base/timestamp.h"
#include "core/inf/variable.h"
#include "core/types/implements.h"
#include "core/types/null.h"

#include "app/base/application_context.h"

namespace ark {

template<typename T> class VariableWrapper final : public Variable<T>, public Wrapper<Variable<T>>, Implements<VariableWrapper<T>, Variable<T>, Wrapper<Variable<T>>> {
public:
    VariableWrapper(const sp<Variable<T>>& delegate) noexcept
        : Wrapper<Variable<T>>(Null::toSafePtr(delegate)), _variable_impl(nullptr) {
    }
    VariableWrapper(T value) noexcept
        : Wrapper<Variable<T>>(sp<typename Variable<T>::Impl>::make(value)), _variable_impl(static_cast<typename Variable<T>::Impl*>(this->_wrapped.get())) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(VariableWrapper);

    virtual T val() override {
        return this->_wrapped->val();
    }

    virtual bool update(uint64_t timestamp) override {
        return this->_wrapped->update(timestamp) || _timestamp.update(timestamp);
    }

    void set(T value) {
        if(_variable_impl)
            _variable_impl->set(std::move(value));
        else {
            deferedUnref();
            _variable_impl = new typename Variable<T>::Impl(std::move(value));
            this->_wrapped = sp<Variable<T>>::adopt(_variable_impl);
            _timestamp.setDirty();
        }
    }

    void set(const sp<Variable<T>>& delegate) {
        deferedUnref();
        DCHECK(delegate.get() != this, "Recursive delegate being set");
        this->_wrapped = Null::toSafePtr(delegate);
        _timestamp.setDirty();
    }

    T fix() {
        T val = this->_wrapped->val();
        set(val);
        return val;
    }

private:
    void deferedUnref() {
        _variable_impl = nullptr;
        if(this->_wrapped)
            Ark::instance().applicationContext()->deferUnref(std::move(this->_wrapped));
    }

private:
    typename Variable<T>::Impl* _variable_impl;
    Timestamp _timestamp;
};

}

#endif
