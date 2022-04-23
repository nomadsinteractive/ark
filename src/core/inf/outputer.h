#ifndef ARK_CORE_INF_OUTPUTER_H_
#define ARK_CORE_INF_OUTPUTER_H_

#include "core/forwarding.h"

namespace ark {

template<typename T> class Outputer {
public:
    virtual ~Outputer() = default;

    virtual void output(T& obj, const sp<Writable>& out) = 0;
};

}

#endif
