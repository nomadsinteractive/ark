#include "graphics/util/vec4_type.h"

#include "core/ark.h"
#include "core/base/clock.h"
#include "core/impl/variable/integral.h"
#include "core/impl/variable/interpolate.h"
#include "core/impl/variable/second_order_dynamics.h"
#include "core/impl/variable/variable_op1.h"
#include "core/impl/variable/variable_op2.h"
#include "core/util/operators.h"

#include "graphics/impl/vec/vec4_impl.h"

namespace ark {

sp<Vec4> Vec4Type::create(const sp<Numeric>& x, const sp<Numeric>& y, const sp<Numeric>& z, const sp<Numeric>& w)
{
    return sp<Vec4Impl>::make(x, y, z, w);
}

sp<Vec4> Vec4Type::create(float x, float y, float z, float w)
{
    return sp<Vec4Impl>::make(x, y, z, w);
}

sp<Vec4> Vec4Type::add(const sp<Vec4>& lvalue, const sp<Vec4>& rvalue)
{
    return sp<VariableOP2<sp<Vec4>, sp<Vec4>, Operators::Add<V4>>>::make(lvalue, rvalue);
}

sp<Vec4> Vec4Type::sub(const sp<Vec4>& lvalue, const sp<Vec4>& rvalue)
{
    return sp<VariableOP2<sp<Vec4>, sp<Vec4>, Operators::Sub<V4>>>::make(lvalue, rvalue);
}

sp<Vec4> Vec4Type::mul(const sp<Vec4>& lvalue, const sp<Vec4>& rvalue)
{
    return sp<VariableOP2<sp<Vec4>, sp<Vec4>, Operators::Mul<V4>>>::make(lvalue, rvalue);
}

sp<Vec4> Vec4Type::truediv(const sp<Vec4>& lvalue, const sp<Vec4>& rvalue)
{
    return sp<VariableOP2<sp<Vec4>, sp<Vec4>, Operators::Div<V4>>>::make(lvalue, rvalue);
}

sp<Vec4> Vec4Type::floordiv(const sp<Vec4>& self, const sp<Vec4>& rvalue)
{
    return sp<VariableOP2<sp<Vec4>, sp<Vec4>, Operators::FloorDiv<V4>>>::make(self, rvalue);
}

sp<Vec4> Vec4Type::negative(const sp<Vec4>& self)
{
    return sp<VariableOP1<V4>>::make(Operators::Neg<V4>(), self);
}

sp<Vec4> Vec4Type::absolute(const sp<Vec4>& self)
{
    return sp<VariableOP1<V4>>::make(Operators::Abs<V4>(), self);
}

sp<Vec4> Vec4Type::transform(const sp<Vec4>& self, const sp<Transform>& transform, const sp<Vec4>& org)
{
    FATAL("Unimplemented");
    return nullptr;
}

sp<Vec4> Vec4Type::integral(const sp<Vec4>& self, const sp<Numeric>& t)
{
    sp<Numeric> duration = t ? t : Ark::instance().appClock()->duration();
    return sp<Integral<V4>>::make(self, std::move(duration));
}

sp<Numeric> Vec4Type::distanceTo(const sp<Vec4>& self, const sp<Vec4>& other)
{
    return Math::distance(self, other);
}

void Vec4Type::set(const sp<VariableWrapper<V4>>& self, const V4& val)
{
    self->set(val);
}

void Vec4Type::set(const sp<VariableWrapper<V4>>& self, const sp<Vec4>& val)
{
    self->set(val);
}

void Vec4Type::set(const sp<Vec4>& self, const V4& val)
{
    ensureImpl(self)->set(val);
}

V4 Vec4Type::val(const sp<Vec4>& self)
{
    return self->val();
}

V3 Vec4Type::xyz(const sp<Vec4>& self)
{
    return self->val();
}

V2 Vec4Type::xy(const sp<Vec4>& self)
{
    return self->val();
}

void Vec4Type::setXy(const sp<Vec4>& self, const V2& xy)
{
    const sp<Vec4Impl>& impl = ensureImpl(self);
    impl->x()->set(xy.x());
    impl->y()->set(xy.y());
}

float Vec4Type::x(const sp<Vec4>& self)
{
    const sp<Vec4Impl> impl = self.as<Vec4Impl>();
    return impl ? impl->x()->val() : self->val().x();
}

void Vec4Type::setX(const sp<Vec4>& self, float x)
{
    ensureImpl(self)->x()->set(x);
}

void Vec4Type::setX(const sp<Vec4>& self, const sp<Numeric>& x)
{
    ensureImpl(self)->x()->set(x);
}

float Vec4Type::y(const sp<Vec4>& self)
{
    const sp<Vec4Impl> impl = self.as<Vec4Impl>();
    return impl ? impl->y()->val() : self->val().y();
}

void Vec4Type::setY(const sp<Vec4>& self, float y)
{
    ensureImpl(self)->y()->set(y);
}

void Vec4Type::setY(const sp<Vec4>& self, const sp<Numeric>& y)
{
    ensureImpl(self)->y()->set(y);
}

float Vec4Type::z(const sp<Vec4>& self)
{
    return self->val().z();
}

void Vec4Type::setZ(const sp<Vec4>& self, float z)
{
    ensureImpl(self)->z()->set(z);
}

void Vec4Type::setZ(const sp<Vec4>& self, const sp<Numeric>& z)
{
    ensureImpl(self)->z()->set(z);
}

float Vec4Type::w(const sp<Vec4>& self)
{
    const sp<Vec4Impl> impl = self.as<Vec4Impl>();
    return impl ? impl->w()->val() : self->val().w();
}

void Vec4Type::setW(const sp<Vec4>& self, float w)
{
    ensureImpl(self)->w()->set(w);
}

void Vec4Type::setW(const sp<Vec4>& self, const sp<Numeric>& w)
{
    ensureImpl(self)->w()->set(w);
}

sp<Numeric> Vec4Type::vx(const sp<Vec4>& self)
{
    const sp<Vec4Impl> impl = self.as<Vec4Impl>();
    return impl ? static_cast<sp<Numeric>>(impl->x()) : sp<Numeric>::make<VariableOP1<float, V4>>(Operators::RandomAccess<V4, float>(0), self);
}

sp<Numeric> Vec4Type::vy(const sp<Vec4>& self)
{
    const sp<Vec4Impl> impl = self.as<Vec4Impl>();
    return impl ? static_cast<sp<Numeric>>(impl->y()) : sp<Numeric>::make<VariableOP1<float, V4>>(Operators::RandomAccess<V4, float>(1), self);
}

sp<Numeric> Vec4Type::vz(const sp<Vec4>& self)
{
    const sp<Vec4Impl> impl = self.as<Vec4Impl>();
    return impl ? static_cast<sp<Numeric>>(impl->z()) : sp<Numeric>::make<VariableOP1<float, V4>>(Operators::RandomAccess<V4, float>(2), self);
}

sp<Numeric> Vec4Type::vw(const sp<Vec4>& self)
{
    const sp<Vec4Impl> impl = self.as<Vec4Impl>();
    return impl ? static_cast<sp<Numeric>>(impl->w()) : sp<Numeric>::make<VariableOP1<float, V4>>(Operators::RandomAccess<V4, float>(3), self);
}

void Vec4Type::fix(const sp<Vec4>& self)
{
    ensureImpl(self)->fix();
}

sp<Vec4> Vec4Type::freeze(const sp<Vec4>& self)
{
    return sp<Vec4::Const>::make(self->val());
}

sp<Vec4> Vec4Type::wrap(const sp<Vec4>& self)
{
    return sp<VariableWrapper<V4>>::make(self);
}

sp<Vec4> Vec4Type::synchronize(const sp<Vec4>& self, const sp<Boolean>& disposed)
{
    return Ark::instance().applicationContext()->synchronize(self, disposed);
}

sp<Vec4> Vec4Type::modFloor(const sp<Vec4>& self, const sp<Numeric>& mod)
{
    return sp<VariableOP2<sp<Vec4>, sp<Vec4>, Operators::ModFloor<V4>>>::make(self, create(mod, mod, mod, mod));
}

sp<Vec4> Vec4Type::modFloor(const sp<Vec4>& self, const sp<Vec4>& mod)
{
    return sp<VariableOP2<sp<Vec4>, sp<Vec4>, Operators::ModFloor<V4>>>::make(self, mod);
}

sp<Vec4> Vec4Type::modCeil(const sp<Vec4>& self, const sp<Numeric>& mod)
{
    return sp<VariableOP2<sp<Vec4>, sp<Vec4>, Operators::ModCeil<V4>>>::make(self, create(mod, mod, mod, mod));
}

sp<Vec4> Vec4Type::modCeil(const sp<Vec4>& self, const sp<Vec4>& mod)
{
    return sp<VariableOP2<sp<Vec4>, sp<Vec4>, Operators::ModCeil<V4>>>::make(self, mod);
}

sp<Vec4> Vec4Type::lerp(const sp<Vec4>& self, const sp<Vec4>& b, const sp<Numeric>& t)
{
    return sp<Interpolate<V4, float>>::make(self, b, t);
}

sp<Vec4> Vec4Type::sod(sp<Vec4> self, float k, float z, float r, sp<Numeric> t)
{
    if(t == nullptr)
        t = Ark::instance().appClock()->duration();
    return sp<SecondOrderDynamics<V4>>::make(std::move(self), std::move(t), k, z, r);
}

sp<Vec4Impl> Vec4Type::ensureImpl(const sp<Vec4>& self)
{
    const sp<Vec4Impl> impl = self.as<Vec4Impl>();
    DCHECK(impl, "This Vec4 object is not a Vec4Impl instance");
    return impl;
}

template<> ARK_API sp<Vec4> Null::ptr()
{
    return sp<Vec4Impl>::make();
}

}
