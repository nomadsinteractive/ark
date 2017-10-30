#ifndef ARK_GRAPHICS_INF_BLOCK_H_
#define ARK_GRAPHICS_INF_BLOCK_H_

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API Block {
public:
    virtual ~Block() = default;

    virtual const sp<Size>& size() = 0;
};

}

#endif
