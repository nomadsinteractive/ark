#ifndef ARK_GRAPHICS_INF_BLOCK_H_
#define ARK_GRAPHICS_INF_BLOCK_H_

#include "core/forwarding.h"
#include "core/base/api.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API Block {
public:
    virtual ~Block() = default;

    virtual const SafePtr<Size>& size() = 0;
};

}

#endif
