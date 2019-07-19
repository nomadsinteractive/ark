#ifndef ARK_CORE_INF_IMPORTER_H_
#define ARK_CORE_INF_IMPORTER_H_

#include "core/forwarding.h"

namespace ark {

template<typename T> class Importer {
public:
    virtual ~Importer() = default;

    virtual void import(T& obj, const sp<Readable>& src) = 0;
};

}

#endif
