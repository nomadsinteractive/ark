#ifndef ARK_CORE_TYPES_SAFE_VAR_H_
#define ARK_CORE_TYPES_SAFE_VAR_H_

#include "core/inf/variable.h"
#include "core/base/wrapper.h"
#include "core/types/null.h"
#include "core/types/shared_ptr.h"

namespace ark {

class _SafeVarDefaultUpdater {
public:
    void operator() () const {}

};

template<typename T> class SafeVar : public Wrapper<T> {
public:
    typedef decltype(std::declval<T>().val()) ValType;

    SafeVar() noexcept
        : Wrapper<T>(nullptr) {
    }
    SafeVar(sp<T> delegate) noexcept
        : Wrapper<T>(std::move(delegate)) {
    }
    SafeVar(sp<T> delegate, const ValType& defaultVal) noexcept
        : Wrapper<T>(std::move(delegate)), _default_val(defaultVal) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(SafeVar);

    explicit operator bool() const {
        return static_cast<bool>(this->_wrapped);
    }

    bool operator == (const sp<T>& other) const {
        return this->_wrapped != other;
    }
    bool operator != (const sp<T>& other) const {
        return this->_wrapped != other;
    }

    ValType val() const {
        return this->_wrapped ? this->_wrapped->val() : _default_val;
    }

    bool update(uint64_t timestamp) const {
        return this->_wrapped ? this->_wrapped->update(timestamp) : false;
    }

    template<typename UPDATER = _SafeVarDefaultUpdater> const sp<T>& ensure(const UPDATER& updater = UPDATER()) {
        if(!this->_wrapped) {
            this->_wrapped = ensure_sfinae(nullptr);
            updater();
        }
        return this->_wrapped;
    }

private:
    template<typename U = T> sp<T> ensure_sfinae(std::enable_if_t<std::is_abstract_v<U>, typename U::Const>*) const {
        return sp<typename U::Const>::make(_default_val);
    }

    template<typename U = T> sp<T> ensure_sfinae(std::enable_if_t<!std::is_abstract_v<U>>*) const {
        return sp<U>::make(_default_val);
    }

    template<typename U = T> sp<T> ensure_sfinae(...) const {
        DFATAL("Constructor for default value required");
        return Null::toSafe<U>(nullptr);
    }

private:
    ValType _default_val;
};

}

#endif
