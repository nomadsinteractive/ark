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
};

template<typename T> class Builder<T>::Null final : public Builder<T> {
public:
    virtual sp<T> build(const Scope& /*args*/) {
        return nullptr;
    }
};

template<typename T> class Builder<T>::Prebuilt final : public Builder<T> {
public:
    Prebuilt(sp<T> instance)
        : _instance(std::move(instance)) {
    }

    virtual sp<T> build(const Scope& /*args*/) {
        return _instance;
    }

private:
    sp<T> _instance;
};

template<typename T> template<typename U> class Builder<T>::Wrapper : public Builder<T> {
public:
    Wrapper(const sp<Builder<U>>& delegate)
        : _delegate(delegate) {
    }

    virtual sp<T> build(const Scope& args) {
        return _delegate->build(args);
    }

private:
    sp<Builder<U>> _delegate;
};

}
