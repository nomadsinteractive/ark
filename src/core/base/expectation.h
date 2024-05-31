#pragma once

#include <vector>

#include "core/base/api.h"
#include "core/base/wrapper.h"
#include "core/base/observer.h"
#include "core/base/notifier.h"
#include "core/inf/holder.h"
#include "core/inf/variable.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class Expectation : public Variable<T>, public Wrapper<Variable<T>>, public Holder, public Implements<Expectation<T>, Variable<T>, Wrapper<Variable<T>>, Holder> {
public:
    Expectation(sp<Variable<T>> delegate, Notifier notifier)
        : Wrapper<Variable<T>>(std::move(delegate)), _notifier(std::move(notifier)) {
    }

    T val() override {
        return this->_wrapped->val();
    }

    bool update(uint64_t timestamp) override {
        return this->_wrapped->update(timestamp);
    }

    void traverse(const Visitor& visitor) override {
        for(const sp<Observer>& i : _observers)
            i->traverse(visitor);
    }

    sp<Observer> createObserver(const sp<Runnable>& callback, bool oneshot = true) {
        return _notifier.createObserver(callback, oneshot);
    }

    const sp<Observer>& addObserver(const sp<Runnable>& callback, bool oneshot = true) {
        _observers.push_back(_notifier.createObserver(callback, oneshot));
        return _observers.back();
    }

    void clear() {
        _observers.clear();
    }

private:
    Notifier _notifier;
    std::vector<sp<Observer>> _observers;
};

}
