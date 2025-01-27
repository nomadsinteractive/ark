#pragma once

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class IBuilder {
public:
    virtual ~IBuilder() = default;

    virtual T build(const Scope& args) = 0;

    class Null;
    class Prebuilt;
};

template<typename T> class IBuilder<T>::Null final : public IBuilder {
public:
    T build(const Scope& /*args*/) override {
        return {};
    }
};

template<typename T> class IBuilder<T>::Prebuilt final : public IBuilder {
public:
    Prebuilt(T instance)
        : _instance(std::move(instance)) {
    }

    T build(const Scope& /*args*/) override {
        return _instance;
    }

private:
    T _instance;
};

}
