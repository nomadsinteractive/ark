#ifndef ARK_CORE_TYPES_SAFE_VAR_H_
#define ARK_CORE_TYPES_SAFE_VAR_H_

#include "core/inf/variable.h"
#include "core/base/delegate.h"
#include "core/types/null.h"
#include "core/types/shared_ptr.h"

namespace ark {

class _SafeVarDefaultUpdater {
public:
    void operator() () {}

};

template<typename T, typename UPDATER = _SafeVarDefaultUpdater> class SafeVar : public Delegate<T> {
public:
    typedef decltype(std::declval<T>().val()) ValType;

    SafeVar(const sp<T>& delegate) noexcept
        : Delegate<T>(delegate) {
    }
    SafeVar(const sp<T>& delegate, const ValType& defaultVal) noexcept
        : Delegate<T>(delegate), _default_val(defaultVal) {
    }
    SafeVar(const sp<T>& delegate, UPDATER updater) noexcept
        : Delegate<T>(delegate), _updater(std::move(updater)) {
    }
    SafeVar(const sp<T>& delegate, const ValType& defaultVal, UPDATER updater) noexcept
        : Delegate<T>(delegate), _default_val(defaultVal), _updater(std::move(updater)) {
    }

    explicit operator bool() const {
        return static_cast<bool>(this->_delegate);
    }

    bool operator == (const sp<T>& other) const {
        return this->_delegate != other;
    }
    bool operator != (const sp<T>& other) const {
        return this->_delegate != other;
    }

    SafeVar& operator =(const sp<T>& other) noexcept {
        this->_delegate = other;
        this->_updater();
        return *this;
    }
    SafeVar& operator =(sp<T>&& other) noexcept {
        this->_delegate = std::move(other);
        this->_updater();
        return *this;
    }

    ValType val() const {
        return this->_delegate ? this->_delegate->val() : _default_val;
    }

    bool update(uint64_t timestamp) const {
        return this->_delegate ? this->_delegate->update(timestamp) : false;
    }

    const sp<T>& ensure() {
        if(!this->_delegate) {
            this->_delegate = Null::toSafe<T>(nullptr);
            this->_updater();
        }
        return this->_delegate;
    }

private:
    ValType _default_val;
    UPDATER _updater;
};

}

#endif
