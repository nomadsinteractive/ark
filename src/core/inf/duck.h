#ifndef ARK_CORE_INF_DUCK_H_
#define ARK_CORE_INF_DUCK_H_

#include "core/forwarding.h"

namespace ark {

template<typename T> class Duck {
public:
    virtual ~Duck() = default;

    virtual void to(sp<T>& inst) = 0;

};

}

#endif
