#ifndef ARK_CORE_INF_ASSET_H_
#define ARK_CORE_INF_ASSET_H_

#include "core/forwarding.h"

namespace ark {

class Asset {
public:
    virtual ~Asset() = default;

    virtual sp<Readable> open() = 0;
    virtual String location() = 0;
};

}

#endif
