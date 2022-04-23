#ifndef ARK_CORE_INF_INPUT_H_
#define ARK_CORE_INF_INPUT_H_

#include "core/base/api.h"
#include "core/inf/updatable.h"

namespace ark {

class ARK_API Input : public Updatable {
public:
    virtual ~Input() override = default;

    virtual void flat(void* buf) = 0;
    virtual uint32_t size() = 0;
};

}

#endif
