#pragma once

#include "core/inf/variable.h"
#include "core/base/timestamp.h"
#include "core/base/wrapper.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class VariableDirtyMark final : public Wrapper<Variable<T>>, public Variable<T> {
public:
    VariableDirtyMark(sp<Variable<T>> delegate, Wrapper<Variable<T>>& wrapper)
        : Wrapper<Variable<T>>(std::move(delegate)), _wrapper(wrapper) {
        _timestamp.markDirty();
    }

    T val() override {
        return this->_wrapped->val();
    }

    bool update(uint64_t timestamp) override {
        this->_wrapped->update(timestamp);
        if(_wrapper.wrapped().get() == this) {
            sp<Variable<T>> delegate = std::move(this->_wrapped);
            _wrapper.reset(std::move(delegate));
        }
        else
            WARN("Trying to overwrite a wrapper which doesn't contain myself");
        return _timestamp.update(timestamp);
    }

    static void markDirty(Wrapper<Variable<T>>& wrapper, sp<Variable<T>> delegate) {
        wrapper.reset(sp<Variable<T>>::template make<VariableDirtyMark>(std::move(delegate), wrapper));
    }

private:
    Wrapper<Variable<T>>& _wrapper;
    Timestamp _timestamp;
};

}
