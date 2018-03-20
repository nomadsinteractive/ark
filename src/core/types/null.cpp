#include "core/types/null.h"

#include "core/ark.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<> ARK_API const sp<Numeric> Null::ptr()
{
    return Ark::instance().obtain<Numeric::Const>(0.0f);
}

template<> ARK_API const sp<Integer> Null::ptr()
{
    return Ark::instance().obtain<Integer::Const>(0);
}

template<> ARK_API const sp<Boolean> Null::ptr()
{
    return Ark::instance().obtain<Boolean::Const>(false);
}

}
