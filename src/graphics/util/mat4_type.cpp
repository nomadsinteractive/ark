#include "graphics/util/mat4_type.h"

#include "core/ark.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/impl/variable/variable_op2.h"
#include "core/util/operators.h"
#include "core/util/variable_util.h"

#include "graphics/base/mat.h"
#include "graphics/impl/mat/mat4_impl.h"

namespace ark {

sp<Mat4> Mat4Type::create(const sp<Vec4>& t, const sp<Vec4>& b, const sp<Vec4>& n, const sp<Vec4>& w)
{
    DASSERT((t && b && n && w) || (!t && !b && !n && !w));
    return t ? sp<Mat4>::make<Mat4Impl>(t, b, n, w) : sp<Mat4>::make<Mat4::Const>(M4::identity());
}

sp<Mat4> Mat4Type::create(const V4& t, const V4& b, const V4& n, const V4& w)
{
    return sp<Mat4Impl>::make(t, b, n, w);
}

sp<Mat4> Mat4Type::mul(const sp<Mat4>& lvalue, const sp<Mat4>& rvalue)
{
    DWARN(false, "\"*\" operator on Mat4 has been deprecated, use \"@\" operator");
    return matmul(lvalue, rvalue);
}

sp<Mat4> Mat4Type::matmul(const sp<Mat4>& lvalue, const sp<Mat4>& rvalue)
{
    return sp<VariableOP2<sp<Mat4>, sp<Mat4>, Operators::Mul<M4, M4>>>::make(lvalue, rvalue);
}

sp<Vec4> Mat4Type::mul(const sp<Mat4>& lvalue, const sp<Vec4>& rvalue)
{
    DWARN(false, "\"*\" operator on Mat4 has been deprecated, use \"@\" operator");
    return matmul(lvalue, rvalue);
}

sp<Vec4> Mat4Type::matmul(const sp<Mat4>& lvalue, const sp<Vec4>& rvalue)
{
    return sp<VariableOP2<sp<Mat4>, sp<Vec4>, Operators::Mul<M4, V4>>>::make(lvalue, rvalue);
}

sp<Vec4> Mat4Type::mul(const sp<Mat4>& lvalue, const V4& rvalue)
{
    DWARN(false, "\"*\" operator on Mat4 has been deprecated, use \"@\" operator");
    return matmul(lvalue, rvalue);
}

sp<Vec4> Mat4Type::matmul(const sp<Mat4>& lvalue, const V4& rvalue)
{
    return sp<VariableOP2<sp<Mat4>, V4, Operators::Mul<M4, V4>>>::make(lvalue, rvalue);
}

sp<Vec3> Mat4Type::mul(const sp<Mat4>& lvalue, const sp<Vec3>& rvalue)
{
    DWARN(false, "\"*\" operator on Mat4 has been deprecated, use \"@\" operator");
    return matmul(lvalue, rvalue);
}

sp<Vec3> Mat4Type::matmul(const sp<Mat4>& lvalue, const sp<Vec3>& rvalue)
{
    return sp<VariableOP2<sp<Mat4>, sp<Vec3>, Operators::Mul<M4, V3>>>::make(lvalue, rvalue);
}

sp<Vec3> Mat4Type::mul(const sp<Mat4>& lvalue, const V3& rvalue)
{
    DWARN(false, "\"*\" operator on Mat4 has been deprecated, use \"@\" operator");
    return matmul(lvalue, rvalue);
}

sp<Vec3> Mat4Type::matmul(const sp<Mat4>& lvalue, const V3& rvalue)
{
    return sp<VariableOP2<sp<Mat4>, V3, Operators::Mul<M4, V3>>>::make(lvalue, rvalue);
}

sp<Mat4> Mat4Type::identity()
{
    return sp<Mat4Impl>::make();
}

void Mat4Type::fix(const sp<Mat4>& self)
{
    ensureImpl(self)->fix();
}

sp<Mat4Impl> Mat4Type::ensureImpl(const sp<Mat4>& self)
{
    const sp<Mat4Impl> impl = self.as<Mat4Impl>();
    DCHECK(impl, "This Mat4 object is not a Mat4Impl instance");
    return impl;
}

template<> ARK_API sp<Mat4> Null::ptr()
{
    return sp<Mat4Impl>::make();
}

}
