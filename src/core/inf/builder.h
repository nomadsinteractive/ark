#ifndef ARK_CORE_INF_BUILDER_H_
#define ARK_CORE_INF_BUILDER_H_

#include "core/forwarding.h"

namespace ark {

template<typename T> class Builder {
public:
    virtual ~Builder() = default;

    virtual sp<T> build(const sp<Scope>& args) = 0;
};

}

#endif
