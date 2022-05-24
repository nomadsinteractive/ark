#ifndef ARK_CORE_INF_LOADER_H_
#define ARK_CORE_INF_LOADER_H_

#include "core/forwarding.h"

namespace ark {

template<class T> class Loader {
public:
    virtual ~Loader() = default;

    virtual T load(const sp<Readable>& readable) = 0;

};

}

#endif
