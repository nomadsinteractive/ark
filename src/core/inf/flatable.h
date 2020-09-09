#ifndef ARK_CORE_INF_FLATABLE_H_
#define ARK_CORE_INF_FLATABLE_H_

#include "core/base/api.h"
#include "core/inf/updatable.h"

namespace ark {

class ARK_API Flatable : public Updatable {
public:
    virtual ~Flatable() = default;

    virtual void flat(void* buf) = 0;
    virtual uint32_t size() = 0;
};

}

#endif
