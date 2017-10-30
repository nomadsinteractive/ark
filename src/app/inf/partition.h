#ifndef ARK_APP_INF_PARTITION_H_
#define ARK_APP_INF_PARTITION_H_

#include "core/forwarding.h"

#include "graphics/forwarding.h"

namespace ark {

class Partition {
public:
    virtual ~Partition() = default;

//  [[script::bindings::auto]]
    virtual uint32_t addPoint(const sp<VV2>& pos) = 0;
//  [[script::bindings::auto]]
    virtual void updatePoint(uint32_t id) = 0;
//  [[script::bindings::auto]]
    virtual void removePoint(uint32_t id) = 0;
//  [[script::bindings::auto]]
    virtual List<uint32_t> within(float x, float y, float radius) = 0;
};

}

#endif
