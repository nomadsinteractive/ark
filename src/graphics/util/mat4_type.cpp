#include "graphics/util/mat4_type.h"

#include "core/ark.h"
#include "core/impl/variable/variable_cached.h"
#include "core/impl/variable/variable_dyed.h"
#include "core/impl/variable/variable_op1.h"
#include "core/impl/variable/variable_op2.h"
#include "core/util/operators.h"
#include "core/util/strings.h"
#include "core/util/string_convert.h"

#include "graphics/base/mat.h"
#include "graphics/impl/mat/mat4_impl.h"

namespace ark {

namespace {

struct MatrixOperators {

    struct ToM4 {
        M4 operator()(const M3& m3) const {
            M4 m4 = M4::identity();
            *reinterpret_cast<V3*>(&m4[0]) = *reinterpret_cast<const V3*>(&m3[0]);
            *reinterpret_cast<V3*>(&m4[4]) = *reinterpret_cast<const V3*>(&m3[3]);
            *reinterpret_cast<V3*>(&m4[8]) = *reinterpret_cast<const V3*>(&m3[6]);
            return m4;
        }
    };

    struct Inverse {
        M4 operator()(const M4& matrix) const {
            return MatrixUtil::inverse(matrix);
        }
    };

    struct Rotate {
        M4 operator()(const M4& v1, const V4& v2) const {
            return MatrixUtil::rotate(v1, v2);
        }
    };

    struct Scale {
        M4 operator()(const M4& v1, const V3& v2) const {
            return MatrixUtil::scale(v1, v2);
        }
    };

    struct Translate {
        M4 operator()(const M4& v1, const V3& v2) const {
            return MatrixUtil::translate(v1, v2);
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
    self->update(Timestamp::now());
    return self->val();
}

sp<Mat4> Mat4Type::create(const M4& other)
{
    return sp<Mat4>::make<Mat4Impl>(other);
}

sp<Mat4> Mat4Type::create(sp<Mat3> other)
{
    return sp<Mat4>::make<VariableOP1<M4, M3>>(MatrixOperators::ToM4(), std::move(other));
}

sp<Mat4> Mat4Type::create(sp<Mat4> other)
{
    return sp<Mat4>::make<Mat4Impl>(std::move(other));
}

sp<Mat4> Mat4Type::create(const V4& t, const V4& b, const V4& n, const V4& w)
{
    return sp<Mat4>::make<Mat4Impl>(t, b, n, w);
}

sp<Mat4> Mat4Type::matmul(sp<Mat4> lvalue, sp<Mat4> rvalue)
{
    return sp<Mat4>::make<VariableOP2<sp<Mat4>, sp<Mat4>, Operators::Mul<M4>>>(std::move(lvalue), std::move(rvalue));
}

sp<Mat4> Mat4Type::matmul(sp<Mat4> lvalue, const M4& rvalue)
{
    return sp<Mat4>::make<VariableOP2<sp<Mat4>, M4, Operators::Mul<M4>>>(std::move(lvalue), rvalue);
}

sp<Mat4> Mat4Type::matmul(const M4& lvalue, sp<Mat4> rvalue)
{
    return sp<Mat4>::make<VariableOP2<M4, sp<Mat4>, Operators::Mul<M4>>>(std::move(lvalue), rvalue);
}

sp<Vec4> Mat4Type::matmul(sp<Mat4> lvalue, sp<Vec4> rvalue)
{
    return sp<Vec4>::make<VariableOP2<sp<Mat4>, sp<Vec4>, Operators::Mul<M4, V4>>>(std::move(lvalue), std::move(rvalue));
}

sp<Vec4> Mat4Type::matmul(sp<Mat4> lvalue, const V4& rvalue)
{
    return sp<Vec4>::make<VariableOP2<sp<Mat4>, V4, Operators::Mul<M4, V4>>>(std::move(lvalue), rvalue);
}

sp<Vec3> Mat4Type::matmul(sp<Mat4> lvalue, sp<Vec3> rvalue)
{
    return sp<Vec3>::make<VariableOP2<sp<Mat4>, sp<Vec3>, Operators::Mul<M4, V3>>>(std::move(lvalue), std::move(rvalue));
}

sp<Vec3> Mat4Type::matmul(sp<Mat4> lvalue, const V3& rvalue)
{
    return sp<Vec3>::make<VariableOP2<sp<Mat4>, V3, Operators::Mul<M4, V3>>>(std::move(lvalue), rvalue);
}

sp<Mat4> Mat4Type::rotate(sp<Mat4> self, sp<Vec4> quaternion)
{
    return sp<Mat4>::make<VariableOP2<sp<Mat4>, sp<Vec4>, MatrixOperators::Rotate>>(std::move(self), std::move(quaternion));
}

sp<Mat4> Mat4Type::scale(sp<Mat4> self, sp<Vec3> scale)
{
    return sp<Mat4>::make<VariableOP2<sp<Mat4>, sp<Vec3>, MatrixOperators::Scale>>(std::move(self), std::move(scale));
}

sp<Mat4> Mat4Type::translate(sp<Mat4> self, sp<Vec3> translation)
{
    return sp<Mat4>::make<VariableOP2<sp<Mat4>, sp<Vec3>, MatrixOperators::Translate>>(std::move(self), std::move(translation));
}

sp<Mat4> Mat4Type::inverse(sp<Mat4> self)
{
    return sp<Mat4>::make<VariableCached<M4>>(sp<Mat4>::make<VariableOP1<M4>>(MatrixOperators::Inverse(), std::move(self)));
}

sp<Mat4> Mat4Type::freeze(const sp<Mat4>& self)
{
    return sp<Mat4>::make<Mat4::Const>(val(self));
}

sp<Mat4> Mat4Type::dye(sp<Mat4> self, sp<Boolean> condition, String message)
{
    return sp<Mat4>::make<VariableDyed<M4>>(std::move(self), std::move(condition), std::move(message));
}

sp<Mat4Impl> Mat4Type::ensureImpl(const sp<Mat4>& self)
{
    return self.ensureInstance<Mat4Impl>("This Mat4 object is not a Mat4Impl instance");
}

template<> String StringConvert::repr<M4>(const M4& obj)
{
    const float* buf = obj.value();
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
