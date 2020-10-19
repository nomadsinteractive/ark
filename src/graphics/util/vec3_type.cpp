#include "graphics/util/vec3_type.h"

#include <glm/glm.hpp>

#include "core/ark.h"
#include "core/base/clock.h"
#include "core/impl/variable/integral.h"
#include "core/impl/variable/variable_op2.h"
#include "core/util/operators.h"
#include "core/util/variable_util.h"

#include "graphics/impl/vec/vec3_impl.h"
#include "graphics/impl/vec/vec_neg.h"

namespace ark {

namespace {

class _Vec3Numeric : public Numeric {
public:
    _Vec3Numeric(const sp<Vec3>& delegate, int32_t dim)
        : _delegate(delegate), _dim(dim) {
    }

    virtual float val() override {
        return _delegate->val()[_dim];
    }

    virtual bool update(uint64_t timestamp) override {
        return _delegate->update(timestamp);
    }

private:
    sp<Vec3> _delegate;
    int32_t _dim;
};

class Vec3Cross : public Vec3 {
public:
    Vec3Cross(const sp<Vec3>& a, const sp<Vec3>& b)
        : _a(a), _b(b) {
    }

    virtual V3 val() override {
        return _a->val().cross(_b->val());
    }

    virtual bool update(uint64_t timestamp) override {
        return VariableUtil::update(timestamp, _a, _b);
    }

private:
    sp<Vec3> _a;
    sp<Vec3> _b;

};

class Vec3Normalize : public Delegate<Vec3>, public Vec3, Implements<Vec3Normalize, Vec3, Delegate<Vec3>> {
public:
    Vec3Normalize(const sp<Vec3>& delegate)
        : Delegate(delegate) {
    }

    virtual V3 val() override {
        const V3 v = _delegate->val();
        const glm::vec3 normalized = glm::normalize(glm::vec3(v.x(), v.y(), v.z()));
        return V3(normalized.x, normalized.y, normalized.z);
    }

    virtual bool update(uint64_t timestamp) override {
        return _delegate->update(timestamp);
    }

};

class Vec2ToVec3 : public Vec3, public Holder, Implements<Vec2ToVec3, Vec3, Holder> {
public:
    Vec2ToVec3(const sp<Vec2>& vec2)
        : _vec2(vec2) {
    }

    virtual V3 val() override {
        return V3(_vec2->val(), 0);
    }

    virtual bool update(uint64_t timestamp) override {
        return _vec2->update(timestamp);
    }

    virtual void traverse(const Visitor& visitor) override {
        HolderUtil::visit(_vec2, visitor);
    }

private:
    sp<Vec2> _vec2;

};

}

sp<Vec3> Vec3Type::create(const sp<Numeric>& x, const sp<Numeric>& y, const sp<Numeric>& z)
{
    return sp<Vec3Impl>::make(x, y, z);
}

sp<Vec3> Vec3Type::create(float x, float y, float z)
{
    return sp<Vec3Impl>::make(x, y, z);
}

sp<Vec3> Vec3Type::create(const sp<Vec2>& vec2)
{
    return sp<Vec2ToVec3>::make(vec2);
}

sp<Vec3> Vec3Type::add(const sp<Vec3>& lvalue, const V3& rvalue)
{
    return sp<VariableOP2<sp<Vec3>, V3, Operators::Add<V3>>>::make(lvalue, rvalue);
}

sp<Vec3> Vec3Type::add(const sp<Vec3>& lvalue, const sp<Vec3>& rvalue)
{
    return sp<VariableOP2<sp<Vec3>, sp<Vec3>, Operators::Add<V3>>>::make(lvalue, rvalue);
}

sp<Vec3> Vec3Type::sub(const sp<Vec3>& lvalue, const sp<Vec3>& rvalue)
{
    return sp<VariableOP2<sp<Vec3>, sp<Vec3>, Operators::Sub<V3>>>::make(lvalue, rvalue);
}

sp<Vec3> Vec3Type::mul(const sp<Vec3>& lvalue, const V3& rvalue)
{
    return sp<VariableOP2<sp<Vec3>, V3, Operators::Mul<V3>>>::make(lvalue, rvalue);
}

sp<Vec3> Vec3Type::mul(const sp<Vec3>& lvalue, const sp<Vec3>& rvalue)
{
    return sp<VariableOP2<sp<Vec3>, sp<Vec3>, Operators::Mul<V3>>>::make(lvalue, rvalue);
}

sp<Vec3> Vec3Type::mul(const sp<Vec3>& lvalue, float rvalue)
{
    return sp<VariableOP2<sp<Vec3>, float, Operators::Mul<V3, float>>>::make(lvalue, rvalue);
}

sp<Vec3> Vec3Type::mul(const sp<Vec3>& lvalue, sp<Numeric>& rvalue)
{
    return sp<VariableOP2<sp<Vec3>, sp<Numeric>, Operators::Mul<V3, float>>>::make(lvalue, rvalue);
}

sp<Vec3> Vec3Type::truediv(const sp<Vec3>& lvalue, const V3& rvalue)
{
    return sp<VariableOP2<sp<Vec3>, V3, Operators::Div<V3>>>::make(lvalue, rvalue);
}

sp<Vec3> Vec3Type::truediv(const sp<Vec3>& lvalue, const sp<Vec3>& rvalue)
{
    return sp<VariableOP2<sp<Vec3>, sp<Vec3>, Operators::Div<V3>>>::make(lvalue, rvalue);
}

sp<Vec3> Vec3Type::floordiv(const sp<Vec3>& self, const sp<Vec3>& rvalue)
{
    FATAL("Unimplemented");
    return nullptr;
}

sp<Vec3> Vec3Type::negative(const sp<Vec3>& self)
{
    return sp<VecNeg<V3>>::make(self);
}

void Vec3Type::set(const sp<VariableWrapper<V3>>& self, const V3& val)
{
    self->set(val);
}

void Vec3Type::set(const sp<VariableWrapper<V3>>& self, const sp<Vec3>& val)
{
    self->set(val);
}

void Vec3Type::set(const sp<Vec3>& self, const V3& val)
{
    ensureImpl(self)->set(val);
}

V3 Vec3Type::xyz(const sp<Vec3>& self)
{
    return self->val();
}

void Vec3Type::setXyz(const sp<Vec3>& self, const V3& xyz)
{
    ensureImpl(self)->set(xyz);
}

V2 Vec3Type::xy(const sp<Vec3>& self)
{
    return self->val();
}

void Vec3Type::setXy(const sp<Vec3>& self, const V2& xy)
{
    ensureImpl(self)->set(V3(xy, 0));
}

float Vec3Type::x(const sp<Vec3>& self)
{
    const sp<Vec3Impl> impl = self.as<Vec3Impl>();
    return impl ? impl->x()->val() : self->val().x();
}

void Vec3Type::setX(const sp<Vec3>& self, float x)
{
    ensureImpl(self)->x()->set(x);
}

void Vec3Type::setX(const sp<Vec3>& self, const sp<Numeric>& x)
{
    ensureImpl(self)->x()->set(x);
}

float Vec3Type::y(const sp<Vec3>& self)
{
    const sp<Vec3Impl> impl = self.as<Vec3Impl>();
    return impl ? impl->y()->val() : self->val().y();
}

void Vec3Type::setY(const sp<Vec3>& self, float y)
{
    ensureImpl(self)->y()->set(y);
}

void Vec3Type::setY(const sp<Vec3>& self, const sp<Numeric>& y)
{
    ensureImpl(self)->y()->set(y);
}

float Vec3Type::z(const sp<Vec3>& self)
{
    const sp<Vec3Impl> impl = self.as<Vec3Impl>();
    return impl ? impl->z()->val() : self->val().z();
}

void Vec3Type::setZ(const sp<Vec3>& self, float z)
{
    ensureImpl(self)->z()->set(z);
}

void Vec3Type::setZ(const sp<Vec3>& self, const sp<Numeric>& z)
{
    ensureImpl(self)->z()->set(z);
}

sp<Numeric> Vec3Type::vx(const sp<Vec3>& self)
{
    const sp<Vec3Impl> impl = self.as<Vec3Impl>();
    return impl ? static_cast<sp<Numeric>>(impl->x()) : static_cast<sp<Numeric>>(sp<_Vec3Numeric>::make(self, 0));
}

sp<Numeric> Vec3Type::vy(const sp<Vec3>& self)
{
    const sp<Vec3Impl> impl = self.as<Vec3Impl>();
    return impl ? static_cast<sp<Numeric>>(impl->y()) : static_cast<sp<Numeric>>(sp<_Vec3Numeric>::make(self, 1));
}

sp<Numeric> Vec3Type::vz(const sp<Vec3>& self)
{
    const sp<Vec3Impl> impl = self.as<Vec3Impl>();
    return impl ? static_cast<sp<Numeric>>(impl->z()) : static_cast<sp<Numeric>>(sp<_Vec3Numeric>::make(self, 2));
}

void Vec3Type::fix(const sp<Vec3>& self)
{
    ensureImpl(self)->fix();
}

sp<Vec3> Vec3Type::freeze(const sp<Vec3>& self)
{
    return sp<Vec3::Const>::make(self->val());
}

sp<Vec3> Vec3Type::wrap(const sp<Vec3>& self)
{
    return sp<VariableWrapper<V3>>::make(self);
}

sp<Vec3> Vec3Type::synchronize(const sp<Vec3>& self, const sp<Boolean>& disposed)
{
    return Ark::instance().applicationContext()->synchronize(self, disposed);
}

sp<Vec3> Vec3Type::cross(const sp<Vec3>& self, const sp<Vec3>& other)
{
    return sp<Vec3Cross>::make(self, other);
}

sp<Vec3> Vec3Type::cross(const sp<Vec3>& self, const V3& other)
{
    return sp<Vec3Cross>::make(self, sp<Vec3::Const>::make(other));
}

sp<Vec3> Vec3Type::normalize(const sp<Vec3>& self)
{
    return sp<Vec3Normalize>::make(self);
}

sp<Vec3> Vec3Type::integral(const sp<Vec3>& self, const sp<Numeric>& t)
{
    sp<Numeric> duration = t ? t : Ark::instance().clock()->duration();
    return sp<Integral<V3>>::make(self, std::move(duration));
}

sp<Numeric> Vec3Type::distanceTo(const sp<Vec3>& self, const sp<Vec3>& other)
{
    sp<Vec3> delta = sub(self, other);
    return Math::sqrt(Math::dot(delta, delta));
}

sp<Vec3Impl> Vec3Type::ensureImpl(const sp<Vec3>& self)
{
    const sp<Vec3Impl> impl = self.as<Vec3Impl>();
    DCHECK(impl, "This Vec3 object is not a Vec3Impl instance");
    return impl;
}

template<> ARK_API sp<Vec3> Null::ptr()
{
    return sp<Vec3Impl>::make();
}

}
