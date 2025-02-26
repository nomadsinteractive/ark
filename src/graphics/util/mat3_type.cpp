#include "graphics/util/mat3_type.h"

#include "core/ark.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/impl/variable/variable_op2.h"
#include "core/util/operators.h"
#include "core/util/updatable_util.h"

#include "graphics/base/mat.h"
#include "graphics/impl/mat/mat3_impl.h"

namespace ark {

sp<Mat3> Mat3Type::create(sp<Vec3> t, sp<Vec3> b, sp<Vec3> n)
{
    ASSERT((t && b && n) || (!t && !b && !n));
    return t ? sp<Mat3>::make<Mat3Impl>(std::move(t), std::move(b), std::move(n)) : sp<Mat3>::make<Mat3::Const>(M3::identity());
}

sp<Mat3> Mat3Type::create(const V3& t, const V3& b, const V3& n)
{
    return sp<Mat3Impl>::make(t, b, n);
}

sp<Mat3> Mat3Type::matmul(sp<Mat3> lvalue, sp<Mat3> rvalue)
{
    return sp<VariableOP2<sp<Mat3>, sp<Mat3>, Operators::Mul<M3, M3>>>::make(std::move(lvalue), std::move(rvalue));
}

sp<Vec3> Mat3Type::matmul(sp<Mat3> lvalue, sp<Vec3> rvalue)
{
    return sp<VariableOP2<sp<Mat3>, sp<Vec3>, Operators::Mul<M3, V3>>>::make(std::move(lvalue), std::move(rvalue));
}

sp<Vec3> Mat3Type::matmul(sp<Mat3> lvalue, const V3& rvalue)
{
    return sp<VariableOP2<sp<Mat3>, V3, Operators::Mul<M3, V3>>>::make(std::move(lvalue), rvalue);
}

sp<Vec2> Mat3Type::matmul(sp<Mat3> lvalue, sp<Vec2> rvalue)
{
    return sp<VariableOP2<sp<Mat3>, sp<Vec2>, Operators::Mul<M3, V2>>>::make(std::move(lvalue), std::move(rvalue));
}

sp<Vec2> Mat3Type::matmul(sp<Mat3> lvalue, const V2& rvalue)
{
    return sp<VariableOP2<sp<Mat3>, V2, Operators::Mul<M3, V2>>>::make(std::move(lvalue), rvalue);
}

sp<Mat3> Mat3Type::identity()
{
    return sp<Mat3Impl>::make();
}

sp<Mat3> Mat3Type::freeze(const sp<Mat3>& self)
{
    return sp<Mat3::Const>::make(self->val());
}

sp<Mat3> Mat3Type::ensureImpl(const sp<Mat3>& self)
{
    return self.ensureInstance<Mat3Impl>("This Vec3 object is not a Mat3Impl instance");
}

}
