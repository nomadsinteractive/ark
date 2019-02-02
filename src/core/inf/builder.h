#ifndef ARK_CORE_INF_BUILDER_H_
#define ARK_CORE_INF_BUILDER_H_

#include "core/forwarding.h"

namespace ark {

template<typename T> class Builder {
public:
    virtual ~Builder() = default;

    virtual sp<T> build(const sp<Scope>& args) = 0;

    class Null;

    template<typename U> class Wrapper;
};

template<typename T> class Builder<T>::Null final : public Builder<T> {
public:
    virtual sp<T> build(const sp<Scope>& /*args*/) {
        return nullptr;
    }
};

template<typename T> template<typename U> class Builder<T>::Wrapper : public Builder<T> {
public:
    Wrapper(const sp<Builder<U>>& delegate)
        : _delegate(delegate) {
    }

    virtual sp<T> build(const sp<Scope>& args) {
        return _delegate->build(args);
    }

private:
    sp<Builder<U>> _delegate;
};

}

#endif
