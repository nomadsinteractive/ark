#pragma once

#include "core/forwarding.h"
#include "core/ark.h"
#include "core/base/wrapper.h"
#include "core/base/timestamp.h"
#include "core/inf/variable.h"
#include "core/types/implements.h"

namespace ark {

template<typename T> class VariableWrapper final : public Variable<T>, public Wrapper<Variable<T>>, Implements<VariableWrapper<T>, Variable<T>, Wrapper<Variable<T>>> {
public:
    VariableWrapper(sp<Variable<T>> delegate) noexcept
        : Wrapper<Variable<T>>(std::move(delegate)) {
    }
    VariableWrapper(T value) noexcept
        : Wrapper<Variable<T>>(sp<Variable<T>>::template make<typename Variable<T>::Const>(value)) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(VariableWrapper);

    T val() override {
        return this->_wrapped->val();
    }

    bool update(uint64_t timestamp) override {
        return this->_wrapped->update(timestamp) || _timestamp.update(timestamp);
    }

    void set(T value) {
        this->_wrapped = sp<Variable<T>>::template make<typename Variable<T>::Const>(value);
        _timestamp.markDirty();
    }

    void set(sp<Variable<T>> delegate) {
        DCHECK(delegate.get() != this, "Recursive delegate being set");
        this->_wrapped = std::move(delegate);
        _timestamp.markDirty();
    }

    T fix() {
        T val = this->_wrapped->val();
        set(val);
        return val;
    }

    void deferedUnref() {
        if(this->_wrapped)
            Ark::instance().deferUnref(std::move(this->_wrapped));
    }

private:
    Timestamp _timestamp;
};

}
