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

    bool update(uint32_t tick) override {
        this->_wrapped->update(tick);
        DCHECK(_wrapper.wrapped().get() == this, "Trying to overwrite a wrapper which doesn't contain myself");
        if(!_timestamp.update(tick)) {
            sp<Variable<T>> delegate = std::move(this->_wrapped);
            _wrapper.reset(std::move(delegate));
        }
        return true;
    }

    static void markDirty(Wrapper<Variable<T>>& wrapper, sp<Variable<T>> delegate) {
        wrapper.reset(sp<Variable<T>>::template make<VariableDirtyMark>(std::move(delegate), wrapper));
    }

private:
    Wrapper<Variable<T>>& _wrapper;
    Timestamp _timestamp;
};

}
