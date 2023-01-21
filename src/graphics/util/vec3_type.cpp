#include "graphics/util/vec3_type.h"

#include <glm/glm.hpp>

#include "core/util/operators.h"
#include "core/util/updatable_util.h"

namespace ark {

namespace {

class Vec3Cross : public Vec3 {
public:
    Vec3Cross(const sp<Vec3>& a, const sp<Vec3>& b)
        : _a(a), _b(b) {
    }

    virtual V3 val() override {
        return _a->val().cross(_b->val());
    }

    virtual bool update(uint64_t timestamp) override {
        return UpdatableUtil::update(timestamp, _a, _b);
    }

private:
    sp<Vec3> _a;
    sp<Vec3> _b;

};

class Vec3Normalize : public Wrapper<Vec3>, public Vec3, Implements<Vec3Normalize, Vec3, Wrapper<Vec3>> {
public:
    Vec3Normalize(sp<Vec3> delegate)
        : Wrapper(std::move(delegate)) {
    }

    virtual V3 val() override {
        const V3 v = _wrapped->val();
        const glm::vec3 normalized = glm::normalize(glm::vec3(v.x(), v.y(), v.z()));
        return V3(normalized.x, normalized.y, normalized.z);
    }

    virtual bool update(uint64_t timestamp) override {
        return _wrapped->update(timestamp);
    }

};

}

sp<Vec3> Vec3Type::create(sp<Numeric> x, sp<Numeric> y, sp<Numeric> z)
{
    return sp<Vec3Impl>::make(std::move(x), std::move(y), std::move(z));
}

sp<Vec3> Vec3Type::create(float x, float y, float z)
{
    return sp<Vec3Impl>::make(x, y, z);
}

//sp<Vec3> Vec3Type::add(const sp<Vec3>& lvalue, const V3& rvalue)
//{
//    return sp<VariableOP2<sp<Vec3>, V3, Operators::Add<V3>>>::make(lvalue, rvalue);
//}

//sp<Vec3> Vec3Type::add(const sp<Vec3>& lvalue, const sp<Vec3>& rvalue)
//{
//    return sp<VariableOP2<sp<Vec3>, sp<Vec3>, Operators::Add<V3>>>::make(lvalue, rvalue);
//}

//sp<Vec3> Vec3Type::sub(const sp<Vec3>& lvalue, const sp<Vec3>& rvalue)
//{
//    return sp<VariableOP2<sp<Vec3>, sp<Vec3>, Operators::Sub<V3>>>::make(lvalue, rvalue);
//}

//sp<Vec3> Vec3Type::mul(const sp<Vec3>& lvalue, const V3& rvalue)
//{
//    return sp<VariableOP2<sp<Vec3>, V3, Operators::Mul<V3>>>::make(lvalue, rvalue);
//}

//sp<Vec3> Vec3Type::mul(const sp<Vec3>& lvalue, const sp<Vec3>& rvalue)
//{
//    return sp<VariableOP2<sp<Vec3>, sp<Vec3>, Operators::Mul<V3>>>::make(lvalue, rvalue);
//}

//sp<Vec3> Vec3Type::mul(const sp<Vec3>& lvalue, float rvalue)
//{
//    return sp<VariableOP2<sp<Vec3>, float, Operators::Mul<V3, float>>>::make(lvalue, rvalue);
//}

//sp<Vec3> Vec3Type::mul(const sp<Vec3>& lvalue, sp<Numeric>& rvalue)
//{
//    return sp<VariableOP2<sp<Vec3>, sp<Numeric>, Operators::Mul<V3, float>>>::make(lvalue, rvalue);
//}

//sp<Vec3> Vec3Type::truediv(const sp<Vec3>& lvalue, const sp<Numeric>& rvalue)
//{
//    return sp<VariableOP2<sp<Vec3>, sp<Numeric>, Operators::Div<V3, float>>>::make(lvalue, rvalue);
//}

//sp<Vec3> Vec3Type::truediv(const sp<Vec3>& lvalue, const V3& rvalue)
//{
//    return sp<VariableOP2<sp<Vec3>, V3, Operators::Div<V3>>>::make(lvalue, rvalue);
//}

//sp<Vec3> Vec3Type::truediv(const sp<Vec3>& lvalue, const sp<Vec3>& rvalue)
//{
//    return sp<VariableOP2<sp<Vec3>, sp<Vec3>, Operators::Div<V3>>>::make(lvalue, rvalue);
//}

//sp<Vec3> Vec3Type::floordiv(const sp<Vec3>& self, const sp<Vec3>& rvalue)
//{
//    return sp<VariableOP2<sp<Vec3>, sp<Vec3>, Operators::FloorDiv<V3>>>::make(self, rvalue);
//}

//sp<Vec3> Vec3Type::negative(const sp<Vec3>& self)
//{
//    return sp<VariableOP1<V3>>::make(Operators::Neg<V3>(), self);
//}

//sp<Vec3> Vec3Type::absolute(const sp<Vec3>& self)
//{
//    return sp<VariableOP1<V3>>::make(Operators::Abs<V3>(), self);
//}

//void Vec3Type::set(const sp<VariableWrapper<V3>>& self, const V3& val)
//{
//    self->set(val);
//}

//void Vec3Type::set(const sp<VariableWrapper<V3>>& self, const sp<Vec3>& val)
//{
//    self->set(val);
//}

//void Vec3Type::set(const sp<Vec3>& self, const V3& val)
//{
//    ensureImpl(self)->set(val);
//}

//V3 Vec3Type::val(const sp<Vec3>& self)
//{
//    return self->val();
//}

//V3 Vec3Type::xyz(const sp<Vec3>& self)
//{
//    return self->val();
//}

//void Vec3Type::setXyz(const sp<Vec3>& self, const V3& xyz)
//{
//    ensureImpl(self)->set(xyz);
//}

//V2 Vec3Type::xy(const sp<Vec3>& self)
//{
//    return self->val();
//}

//void Vec3Type::setXy(const sp<Vec3>& self, const V2& xy)
//{
//    ensureImpl(self)->set(V3(xy, self->val().z()));
//}

//float Vec3Type::x(const sp<Vec3>& self)
//{
//    const sp<Vec3Impl> impl = self.as<Vec3Impl>();
//    return impl ? impl->x()->val() : self->val().x();
//}

//void Vec3Type::setX(const sp<Vec3>& self, float x)
//{
//    ensureImpl(self)->x()->set(x);
//}

//void Vec3Type::setX(const sp<Vec3>& self, const sp<Numeric>& x)
//{
//    ensureImpl(self)->x()->set(x);
//}

//float Vec3Type::y(const sp<Vec3>& self)
//{
//    const sp<Vec3Impl> impl = self.as<Vec3Impl>();
//    return impl ? impl->y()->val() : self->val().y();
//}

//void Vec3Type::setY(const sp<Vec3>& self, float y)
//{
//    ensureImpl(self)->y()->set(y);
//}

//void Vec3Type::setY(const sp<Vec3>& self, const sp<Numeric>& y)
//{
//    ensureImpl(self)->y()->set(y);
//}

//float Vec3Type::z(const sp<Vec3>& self)
//{
//    const sp<Vec3Impl> impl = self.as<Vec3Impl>();
//    return impl ? impl->z()->val() : self->val().z();
//}

//void Vec3Type::setZ(const sp<Vec3>& self, float z)
//{
//    ensureImpl(self)->z()->set(z);
//}

//void Vec3Type::setZ(const sp<Vec3>& self, const sp<Numeric>& z)
//{
//    ensureImpl(self)->z()->set(z);
//}

//sp<Numeric> Vec3Type::vx(const sp<Vec3>& self)
//{
//    const sp<Vec3Impl> impl = self.as<Vec3Impl>();
//    return impl ? static_cast<sp<Numeric>>(impl->x()) : sp<Numeric>::make<VariableOP1<float, V3>>(Operators::RandomAccess<V3, float>(0), self);
//}

//sp<Numeric> Vec3Type::vy(const sp<Vec3>& self)
//{
//    const sp<Vec3Impl> impl = self.as<Vec3Impl>();
//    return impl ? static_cast<sp<Numeric>>(impl->y()) : sp<Numeric>::make<VariableOP1<float, V3>>(Operators::RandomAccess<V3, float>(1), self);
//}

//sp<Numeric> Vec3Type::vz(const sp<Vec3>& self)
//{
//    const sp<Vec3Impl> impl = self.as<Vec3Impl>();
//    return impl ? static_cast<sp<Numeric>>(impl->z()) : sp<Numeric>::make<VariableOP1<float, V3>>(Operators::RandomAccess<V3, float>(2), self);
//}

sp<Vec4> Vec3Type::extend(sp<Vec3> self, sp<Numeric> w)
{
    return sp<VariableOP2<sp<Vec3>, sp<Numeric>, Operators::Extend<V3, float>>>::make(std::move(self), std::move(w));
}

//void Vec3Type::fix(const sp<Vec3>& self)
//{
//    ensureImpl(self)->fix();
//}

//sp<Vec3> Vec3Type::freeze(const sp<Vec3>& self)
//{
//    return sp<Vec3::Const>::make(self->val());
//}

//sp<Size> Vec3Type::toSize(const sp<Vec3>& self)
//{
//    return sp<Size>::make(vx(self), vy(self), vz(self));
//}

//sp<Vec3> Vec3Type::wrap(const sp<Vec3>& self)
//{
//    return sp<VariableWrapper<V3>>::make(self);
//}

//sp<Vec3> Vec3Type::synchronize(const sp<Vec3>& self, const sp<Boolean>& disposed)
//{
//    return Ark::instance().applicationContext()->synchronize(self, disposed);
//}

//sp<Vec3> Vec3Type::modFloor(sp<Vec3> self, sp<Numeric> mod)
//{
//    return sp<VariableOP2<sp<Vec3>, sp<Vec3>, Operators::ModFloor<V3>>>::make(std::move(self), create(mod, mod, mod));
//}

//sp<Vec3> Vec3Type::modFloor(sp<Vec3> self, sp<Vec3> mod)
//{
//    return sp<VariableOP2<sp<Vec3>, sp<Vec3>, Operators::ModFloor<V3>>>::make(std::move(self), std::move(mod));
//}

//sp<Vec3> Vec3Type::modCeil(sp<Vec3> self, sp<Numeric> mod)
//{
//    return sp<VariableOP2<sp<Vec3>, sp<Vec3>, Operators::ModCeil<V3>>>::make(self, create(mod, mod, mod));
//}

//sp<Vec3> Vec3Type::modCeil(sp<Vec3> self, sp<Vec3> mod)
//{
//    return sp<VariableOP2<sp<Vec3>, sp<Vec3>, Operators::ModCeil<V3>>>::make(std::move(self), std::move(mod));
//}

//sp<Vec3> Vec3Type::ifElse(sp<Vec3> self, sp<Boolean> condition, sp<Vec3> otherwise)
//{
//    return sp<VariableTernary<V3>>::make(std::move(condition), std::move(self), std::move(otherwise));
//}

//sp<Vec3> Vec3Type::attract(const sp<Vec3>& self, const V3& s0, float duration, const sp<Numeric>& t)
//{
//    sp<Numeric> ts = t ? t : Ark::instance().appClock()->duration();
//    return sp<Vec3Impl>::make(sp<Stalker>::make(ts, vx(self), s0.x(), duration), sp<Stalker>::make(ts, vy(self), s0.y(), duration), sp<Stalker>::make(ts, vz(self), s0.z(), duration));
//}

//sp<Vec3> Vec3Type::lerp(const sp<Vec3>& self, const sp<Vec3>& b, const sp<Numeric>& t)
//{
//    return sp<Interpolate<V3, float>>::make(self, b, t);
//}

//sp<Vec3> Vec3Type::sod(sp<Vec3> self, float k, float z, float r, sp<Numeric> t)
//{
//    if(t == nullptr)
//        t = Ark::instance().appClock()->duration();
//    return sp<SecondOrderDynamics<V3>>::make(std::move(self), std::move(t), k, z, r);
//}

sp<Vec3> Vec3Type::cross(sp<Vec3> self, sp<Vec3> other)
{
    return sp<Vec3Cross>::make(std::move(self), std::move(other));
}

sp<Vec3> Vec3Type::cross(sp<Vec3> self, const V3& other)
{
    return sp<Vec3Cross>::make(std::move(self), sp<Vec3::Const>::make(other));
}

//sp<Vec3> Vec3Type::normalize(const sp<Vec3>& self)
//{
//    return sp<Vec3Normalize>::make(self);
//}

//sp<Vec3> Vec3Type::integral(const sp<Vec3>& self, const sp<Numeric>& t)
//{
//    sp<Numeric> duration = t ? t : Ark::instance().appClock()->duration();
//    return sp<Integral<V3>>::make(self, std::move(duration));
//}

//sp<Numeric> Vec3Type::distanceTo(const sp<Vec3>& self, const sp<Vec3>& other)
//{
//    return Math::distance(self, other);
//}

//sp<Vec3Impl> Vec3Type::ensureImpl(const sp<Vec3>& self)
//{
//    const sp<Vec3Impl> impl = self.as<Vec3Impl>();
//    DCHECK(impl, "This Vec3 object is not a Vec3Impl instance");
//    return impl;
//}

template<> ARK_API sp<Vec3> Null::safePtr()
{
    return sp<Vec3Impl>::make();
}

}
