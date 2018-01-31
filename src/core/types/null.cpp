#include "core/types/null.h"

#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<> ARK_API const sp<Numeric>& Null::ptr()
{
    static const sp<Numeric> inst = sp<Numeric::Const>::make(0.0f);
    return inst;
}

template<> ARK_API const sp<Integer>& Null::ptr()
{
    static const sp<Integer> inst = sp<Integer::Const>::make(0);
    return inst;
}

template<> ARK_API const sp<Boolean>& Null::ptr()
{
    static const sp<Boolean> inst = sp<Boolean::Const>::make(false);
    return inst;
}

}
