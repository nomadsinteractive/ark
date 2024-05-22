#include "core/types/null.h"

#include "core/ark.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<> ARK_API sp<Numeric> Null::safePtr()
{
    return sp<Numeric>::make<Numeric::Const>(0.0f);
}

template<> ARK_API sp<Integer> Null::safePtr()
{
    return sp<Integer>::make<Integer::Const>(0);
}

template<> ARK_API sp<Boolean> Null::safePtr()
{
    return sp<Boolean>::make<Boolean::Const>(false);
}

template<> ARK_API sp<StringVar> Null::safePtr()
{
    return sp<StringVar>::make<StringVar::Const>(nullptr);
}

}
