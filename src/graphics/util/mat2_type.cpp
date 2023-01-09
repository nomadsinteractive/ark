#include "graphics/util/mat2_type.h"

#include "core/ark.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/impl/variable/variable_op2.h"
#include "core/impl/variable/variable_ternary.h"
#include "core/util/operators.h"
#include "core/util/updatable_util.h"

#include "graphics/base/mat.h"
#include "graphics/impl/mat/mat2_impl.h"
#include "graphics/util/matrix_util.h"

namespace ark {

namespace  {

class RotateOP {
public:
    M2 operator()(const M2& v1, float v2) const {
        return MatrixUtil::rotate(v1, v2);
    }
};

}

sp<Mat2> Mat2Type::create(const V2& t, const V2& b)
{
    return sp<Mat2Impl>::make(t, b);
}

sp<Mat2> Mat2Type::create(const sp<Vec2>& t, const sp<Vec2>& b)
{
    DASSERT((t && b) || (!t && !b));
    return t ? sp<Mat2>::make<Mat2Impl>(t, b) : sp<Mat2>::make<Mat2::Const>(M2::identity());
}

sp<Mat2> Mat2Type::mul(const sp<Mat2>& lvalue, const sp<Mat2>& rvalue)
{
    return sp<VariableOP2<sp<Mat2>, sp<Mat2>, Operators::Mul<M2, M2>>>::make(lvalue, rvalue);
}

sp<Vec2> Mat2Type::mul(const sp<Mat2>& lvalue, const sp<Vec2>& rvalue)
{
    return sp<VariableOP2<sp<Mat2>, sp<Vec2>, Operators::Mul<M2, V2>>>::make(lvalue, rvalue);
}

sp<Vec2> Mat2Type::mul(const sp<Mat2>& lvalue, const V2& rvalue)
{
    return sp<VariableOP2<sp<Mat2>, V2, Operators::Mul<M3, V2>>>::make(lvalue, rvalue);
}

sp<Mat2> Mat2Type::rotate(const sp<Mat2>& self, const sp<Numeric>& radians)
{
    return sp<VariableOP2<sp<Mat2>, sp<Numeric>, RotateOP>>::make(self, radians);
}

sp<Mat2> Mat2Type::ifElse(const sp<Mat2>& self, const sp<Boolean>& condition, const sp<Mat2>& negative)
{
    return sp<VariableTernary<M2>>::make(condition, self, negative);
}

sp<Mat2> Mat2Type::freeze(const sp<Mat2>& self)
{
    return sp<Mat2::Const>::make(self->val());
}

template<> ARK_API sp<Mat2> Null::safePtr()
{
    return sp<Mat2Impl>::make();
}

}
