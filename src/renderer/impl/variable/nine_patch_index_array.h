#ifndef ARK_RENDERER_IMPL_VARIABLE_NINE_PATCH_INDEX_ARRAY_H_
#define ARK_RENDERER_IMPL_VARIABLE_NINE_PATCH_INDEX_ARRAY_H_

#include <stdint.h>

#include "core/forwarding.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class NinePatchIndexArray : public Variable<indexarray> {
public:
    NinePatchIndexArray();

    virtual indexarray val() override;

private:
    indexarray _array;
};

}

#endif
