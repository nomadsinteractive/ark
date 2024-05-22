#include "graphics/util/mat4_type.h"

#include "core/ark.h"
#include "core/impl/variable/variable_dyed.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/impl/variable/variable_op2.h"
#include "core/util/operators.h"
#include "core/util/updatable_util.h"

#include "graphics/base/mat.h"
#include "graphics/impl/mat/mat4_impl.h"

namespace ark {

namespace {

class MatrixOperators {
public:

    class Translate {
    public:
        M4 operator()(const M4& v1, const V3& v2) {
            return MatrixUtil::translate(v1, v2);
        }
    };

    class Rotate {
    public:
        M4 operator()(const M4& v1, const V4& v2) {
            return MatrixUtil::rotate(v1, v2);
        }
    };

};

}

sp<Mat4> Mat4Type::create(sp<Vec4> t, sp<Vec4> b, sp<Vec4> n, sp<Vec4> w)
{
    ASSERT((t && b && n && w) || (!t && !b && !n && !w));
    return t ? sp<Mat4>::make<Mat4Impl>(std::move(t), std::move(b), std::move(n), std::move(w)) : sp<Mat4>::make<Mat4::Const>(M4::identity());
}

M4 Mat4Type::val(const sp<Mat4>& self)
{
    return self->val();
}

sp<Mat4> Mat4Type::create(const M4& m)
{
    return sp<Mat4Impl>::make(m);
}

sp<Mat4> Mat4Type::create(sp<Mat4> other)
{
    return sp<Mat4Impl>::make(std::move(other));
}

sp<Mat4> Mat4Type::create(const V4& t, const V4& b, const V4& n, const V4& w)
{
    return sp<Mat4Impl>::make(t, b, n, w);
}

sp<Mat4> Mat4Type::matmul(sp<Mat4> lvalue, sp<Mat4> rvalue)
{
    return sp<VariableOP2<sp<Mat4>, sp<Mat4>, Operators::Mul<M4, M4>>>::make(std::move(lvalue), rvalue);
}

sp<Mat4> Mat4Type::matmul(sp<Mat4> lvalue, const M4& rvalue)
{
    return sp<VariableOP2<sp<Mat4>, M4, Operators::Mul<M4, M4>>>::make(std::move(lvalue), rvalue);
}

sp<Mat4> Mat4Type::matmul(const M4& lvalue, sp<Mat4> rvalue)
{
    return sp<VariableOP2<M4, sp<Mat4>, Operators::Mul<M4, M4>>>::make(std::move(lvalue), rvalue);
}

sp<Vec4> Mat4Type::matmul(sp<Mat4> lvalue, sp<Vec4> rvalue)
{
    return sp<VariableOP2<sp<Mat4>, sp<Vec4>, Operators::Mul<M4, V4>>>::make(std::move(lvalue), std::move(rvalue));
}

sp<Vec4> Mat4Type::matmul(sp<Mat4> lvalue, const V4& rvalue)
{
    return sp<VariableOP2<sp<Mat4>, V4, Operators::Mul<M4, V4>>>::make(std::move(lvalue), rvalue);
}

sp<Vec3> Mat4Type::matmul(sp<Mat4> lvalue, sp<Vec3> rvalue)
{
    return sp<VariableOP2<sp<Mat4>, sp<Vec3>, Operators::Mul<M4, V3>>>::make(std::move(lvalue), std::move(rvalue));
}

sp<Vec3> Mat4Type::matmul(sp<Mat4> lvalue, const V3& rvalue)
{
    return sp<VariableOP2<sp<Mat4>, V3, Operators::Mul<M4, V3>>>::make(std::move(lvalue), rvalue);
}

sp<Mat4> Mat4Type::identity()
{
    return sp<Mat4Impl>::make();
}

sp<Mat4> Mat4Type::translate(sp<Mat4> self, sp<Vec3> translation)
{
    return sp<VariableOP2<sp<Mat4>, sp<Vec3>, MatrixOperators::Translate>>::make(std::move(self), std::move(translation));
}

sp<Mat4> Mat4Type::rotate(sp<Mat4> self, sp<Vec4> quaternion)
{
    return sp<VariableOP2<sp<Mat4>, sp<Vec4>, MatrixOperators::Rotate>>::make(std::move(self), std::move(quaternion));
}

sp<Mat4> Mat4Type::freeze(const sp<Mat4>& self)
{
    return sp<Mat4::Const>::make(self->val());
}

sp<Mat4> Mat4Type::dye(sp<Mat4> self, sp<Boolean> condition, String message)
{
    return sp<VariableDyed<M4>>::make(std::move(self), std::move(condition), std::move(message));
}

sp<Mat4Impl> Mat4Type::ensureImpl(const sp<Mat4>& self)
{
    const sp<Mat4Impl> impl = self.tryCast<Mat4Impl>();
    CHECK(impl, "This Mat4 object is not a Mat4Impl instance");
    return impl;
}

template<> sp<Mat4> Null::safePtr()
{
    return sp<Mat4Impl>::make();
}

template<> String StringConvert::to<M4, String>(const M4& val)
{
    const float* buf = val.value();
    StringBuffer sb;
    sb << "(";
    for(size_t i = 0; i < 4; ++i)
    {
        if(i != 0)
            sb << ", ";
        sb << Strings::sprintf("(%.2f, %.2f, %.2f, %.2f)", buf[0], buf[1], buf[2], buf[3]);
        buf += 4;
    }
    sb << ")";
    return sb.str();
}

}
