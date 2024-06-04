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

template<typename T> class Builder<T>::Null final : public Builder<T> {
public:
    sp<T> build(const Scope& /*args*/) override {
        return nullptr;
    }
};

template<typename T> class Builder<T>::Prebuilt final : public Builder<T> {
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

template<typename T> template<typename U> class Builder<T>::Wrapper final : public Builder<T> {
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

template<typename T> template<typename... Args> class Builder<T>::Lazy final : public Builder<T> {
public:
    Lazy(Args&&... args)
        : _args(std::forward<Args>(args)...) {
    }

    sp<T> build(const Scope& /*args*/) override {
        typedef sp<T> (*pfnMakeSharedPtr)(Args&&...);
        return std::apply(static_cast<pfnMakeSharedPtr>(sp<T>::template make<T, Args...>), _args);
    }

private:
    std::tuple<Args...> _args;
};

}
