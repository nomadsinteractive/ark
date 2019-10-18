#include "core/types/null.h"

#include "core/ark.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<> ARK_API sp<Numeric> Null::ptr()
{
    return sp<Numeric>::make<Numeric::Const>(0.0f);
}

template<> ARK_API sp<Integer> Null::ptr()
{
    return sp<Integer>::make<Integer::Const>(0);
}

template<> ARK_API sp<Boolean> Null::ptr()
{
    return sp<Boolean>::make<Boolean::Const>(false);
}

}
