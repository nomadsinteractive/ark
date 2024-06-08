#pragma once

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class Builder {
public:
    virtual ~Builder() = default;

    virtual sp<T> build(const Scope& args) = 0;

    class Null;

    class Prebuilt;
    template<typename U> class Wrapper;
    template<typename... Args> class Lazy;
};

template<typename T> class Builder<T>::Null final : public Builder {
public:
    sp<T> build(const Scope& /*args*/) override {
        return nullptr;
    }
};

template<typename T> class Builder<T>::Prebuilt final : public Builder {
public:
    Prebuilt(sp<T> instance)
        : _instance(std::move(instance)) {
    }

    sp<T> build(const Scope& /*args*/) override {
        return _instance;
    }

private:
    sp<T> _instance;
};

template<typename T> template<typename U> class Builder<T>::Wrapper final : public Builder {
public:
    Wrapper(sp<Builder<U>> delegate)
        : _delegate(std::move(delegate)) {
    }

    sp<T> build(const Scope& args) override {
        return _delegate->build(args);
    }

private:
    sp<Builder<U>> _delegate;
};

template<typename T> template<typename... Args> class Builder<T>::Lazy final : public Builder {
public:
    typedef std::function<sp<T>(Args&&...)> MakeSharedPtrType;

    Lazy(MakeSharedPtrType makerFunc, Args&&... args)
        : _args(std::forward<Args>(args)...), _maker_func(std::move(makerFunc)) {
    }

    sp<T> build(const Scope& /*args*/) override {
        if(!_lazy_instance) {
            _lazy_instance = std::apply(_maker_func, std::move(_args));
        }
        return _lazy_instance;
    }

private:
    sp<T> _lazy_instance;
    std::tuple<Args...> _args;
    MakeSharedPtrType _maker_func;
};

template<typename T, typename U = T, typename... Args> sp<Builder<T>> make_lazy_builder(Args&&... args) {
    return sp<Builder<T>>::template make<typename Builder<T>::template Lazy<Args...>>(sp<T>::template make<U, Args...>, std::forward<Args>(args)...);
}

template<typename T, typename U, typename... Args> sp<Builder<T>> to_lazy_builder(U makerFunc, Args&&... args) {
    return sp<Builder<T>>::template make<typename Builder<T>::template Lazy<Args...>>(std::move(makerFunc), std::forward<Args>(args)...);
}

}
