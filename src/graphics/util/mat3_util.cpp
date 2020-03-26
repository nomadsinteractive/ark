#include "graphics/util/mat3_util.h"

#include "core/ark.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/impl/variable/variable_op2.h"
#include "core/util/operators.h"
#include "core/util/variable_util.h"

#include "graphics/base/mat.h"
#include "graphics/impl/mat/mat3_impl.h"

namespace ark {

sp<Mat3> Mat3Util::create(const sp<Vec3>& t, const sp<Vec3>& b, const sp<Vec3>& n)
{
    DASSERT((t && b && n) || (!t && !b && !n));
    return t ? sp<Mat3>::make<Mat3Impl>(t, b, n) : sp<Mat3>::make<Mat3::Const>(M3::identity());
}

sp<Mat3> Mat3Util::create(const V3& t, const V3& b, const V3& n)
{
    return sp<Mat3Impl>::make(t, b, n);
}

sp<Mat3> Mat3Util::mul(const sp<Mat3>& lvalue, const sp<Mat3>& rvalue)
{
    return sp<VariableOP2<sp<Mat3>, sp<Mat3>, Operators::Mul<M3, M3>>>::make(lvalue, rvalue);
}

sp<Vec3> Mat3Util::mul(const sp<Mat3>& lvalue, const sp<Vec3>& rvalue)
{
    return sp<VariableOP2<sp<Mat3>, sp<Vec3>, Operators::Mul<M3, V3>>>::make(lvalue, rvalue);
}

sp<Vec3> Mat3Util::mul(const sp<Mat3>& lvalue, const V3& rvalue)
{
    return sp<VariableOP2<sp<Mat3>, V3, Operators::Mul<M3, V3>>>::make(lvalue, rvalue);
}

sp<Vec2> Mat3Util::mul(const sp<Mat3>& lvalue, const sp<Vec2>& rvalue)
{
    return sp<VariableOP2<sp<Mat3>, sp<Vec2>, Operators::Mul<M3, V2>>>::make(lvalue, rvalue);
}

sp<Vec2> Mat3Util::mul(const sp<Mat3>& lvalue, const V2& rvalue)
{
    return sp<VariableOP2<sp<Mat3>, V2, Operators::Mul<M3, V2>>>::make(lvalue, rvalue);
}

sp<Mat3> Mat3Util::identity()
{
    return sp<Mat3Impl>::make();
}

void Mat3Util::fix(const sp<Mat3>& self)
{
    ensureImpl(self)->fix();
}

sp<Mat3Impl> Mat3Util::ensureImpl(const sp<Mat3>& self)
{
    const sp<Mat3Impl> impl = self.as<Mat3Impl>();
    DCHECK(impl, "This Vec3 object is not a Mat3Impl instance");
    return impl;
}

template<> ARK_API sp<Mat3> Null::ptr()
{
    return sp<Mat3Impl>::make();
}

}
