#pragma once

#include "core/inf/variable.h"
#include "core/base/wrapper.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class VariableDirty final : public Variable<T> {
public:
    VariableDirty(sp<Variable<T>> delegate, Wrapper<Variable<T>>& wrapper)
        : _delegate(std::move(delegate)), _wrapper(wrapper) {
    }

    T val() override {
        return _delegate->val();
    }

    bool update(uint64_t timestamp) override {
        _delegate->update(timestamp);
        if(_wrapper.wrapped().get() == this) {
            sp<Variable<T>> delegate = std::move(_delegate);
            _wrapper.reset(std::move(delegate));
        }
        else
            WARN("Trying to overwrite a wrapper which doesn't contain myself");
        return true;
    }

    static void reset(Wrapper<Variable<T>>& wrapper, sp<Variable<T>> delegate) {
        wrapper.reset(sp<Variable<T>>::make<VariableDirty>(std::move(delegate), wrapper));
    }

private:
    sp<Variable<T>> _delegate;
    Wrapper<Variable<T>>& _wrapper;
};

}
