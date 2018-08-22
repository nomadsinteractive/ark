#include "graphics/util/vec3_util.h"

#include <glm/glm.hpp>

#include "core/ark.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/impl/variable/variable_op2.h"
#include "core/util/operators.h"

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

private:
    sp<Vec3> _a;
    sp<Vec3> _b;
};

class Vec3Normalize : public Vec3 {
public:
    Vec3Normalize(const sp<Vec3>& delegate)
        : _delegate(delegate) {
    }

    virtual V3 val() override {
        const V3 v = _delegate->val();
        glm::vec3 normalized = glm::normalize(glm::vec3(v.x(), v.y(), v.z()));
        return V3(normalized.x, normalized.y, normalized.z);
    }

private:
    sp<Vec3> _delegate;
};

}

sp<Vec3> Vec3Util::create(const sp<Numeric>& x, const sp<Numeric>& y, const sp<Numeric>& z)
{
    return sp<Vec3Impl>::make(x, y, z);
}

sp<Vec3> Vec3Util::create(float x, float y, float z)
{
    return sp<Vec3Impl>::make(x, y, z);
}

sp<Vec3> Vec3Util::add(const sp<Vec3>& lvalue, const sp<Vec3>& rvalue)
{
    return sp<VariableOP2<V3, V3, Operators::Add<V3>, sp<Vec3>, sp<Vec3>>>::make(lvalue, rvalue);
}

sp<Vec3> Vec3Util::sub(const sp<Vec3>& lvalue, const sp<Vec3>& rvalue)
{
    return sp<VariableOP2<V3, V3, Operators::Sub<V3>, sp<Vec3>, sp<Vec3>>>::make(lvalue, rvalue);
}

sp<Vec3> Vec3Util::mul(const sp<Vec3>& lvalue, const sp<Vec3>& rvalue)
{
    return sp<VariableOP2<V3, V3, Operators::Mul<V3>, sp<Vec3>, sp<Vec3>>>::make(lvalue, rvalue);
}

sp<Vec3> Vec3Util::mul(const sp<Vec3>& lvalue, const V3& rvalue)
{
    return sp<VariableOP2<V3, V3, Operators::Mul<V3>, sp<Vec3>, V3>>::make(lvalue, rvalue);
}

sp<Vec3> Vec3Util::mul(const sp<Vec3>& lvalue, float rvalue)
{
    return sp<VariableOP2<V3, float, Operators::Mul<V3, float>, sp<Vec3>, float>>::make(lvalue, rvalue);
}

sp<Vec3> Vec3Util::truediv(const sp<Vec3>& lvalue, const sp<Vec3>& rvalue)
{
    return sp<VariableOP2<V3, V3, Operators::Div<V3>, sp<Vec3>, sp<Vec3>>>::make(lvalue, rvalue);
}

sp<Vec3> Vec3Util::floordiv(const sp<Vec3>& self, const sp<Vec3>& rvalue)
{
    FATAL("Unimplemented");
    return nullptr;
}

sp<Vec3> Vec3Util::negative(const sp<Vec3>& self)
{
    return sp<VecNeg<V3>>::make(self);
}

V3 Vec3Util::xyz(const sp<Vec3>& self)
{
    return self->val();
}

V2 Vec3Util::xy(const sp<Vec3>& self)
{
    return self->val();
}

void Vec3Util::setXy(const sp<Vec3>& self, const V2& xy)
{
    ensureImpl(self)->set(xy);
}

float Vec3Util::x(const sp<Vec3>& self)
{
    const sp<Vec3Impl> impl = self.as<Vec3Impl>();
    return impl ? impl->x()->val() : self->val().x();
}

void Vec3Util::setX(const sp<Vec3>& self, float x)
{
    ensureImpl(self)->x()->set(x);
}

void Vec3Util::setX(const sp<Vec3>& self, const sp<Numeric>& x)
{
    ensureImpl(self)->x()->set(x);
}

float Vec3Util::y(const sp<Vec3>& self)
{
    const sp<Vec3Impl> impl = self.as<Vec3Impl>();
    return impl ? impl->y()->val() : self->val().y();
}

void Vec3Util::setY(const sp<Vec3>& self, float y)
{
    ensureImpl(self)->y()->set(y);
}

void Vec3Util::setY(const sp<Vec3>& self, const sp<Numeric>& y)
{
    ensureImpl(self)->y()->set(y);
}

float Vec3Util::z(const sp<Vec3>& self)
{
    const sp<Vec3Impl> impl = self.as<Vec3Impl>();
    return impl ? impl->z()->val() : self->val().z();
}

void Vec3Util::setZ(const sp<Vec3>& self, float z)
{
    ensureImpl(self)->z()->set(z);
}

void Vec3Util::setZ(const sp<Vec3>& self, const sp<Numeric>& z)
{
    ensureImpl(self)->z()->set(z);
}

sp<Numeric> Vec3Util::vx(const sp<Vec3>& self)
{
    const sp<Vec3Impl> impl = self.as<Vec3Impl>();
    return impl ? static_cast<sp<Numeric>>(impl->x()) : static_cast<sp<Numeric>>(sp<_Vec3Numeric>::make(self, 0));
}

sp<Numeric> Vec3Util::vy(const sp<Vec3>& self)
{
    const sp<Vec3Impl> impl = self.as<Vec3Impl>();
    return impl ? static_cast<sp<Numeric>>(impl->y()) : static_cast<sp<Numeric>>(sp<_Vec3Numeric>::make(self, 1));
}

sp<Numeric> Vec3Util::vz(const sp<Vec3>& self)
{
    const sp<Vec3Impl> impl = self.as<Vec3Impl>();
    return impl ? static_cast<sp<Numeric>>(impl->z()) : static_cast<sp<Numeric>>(sp<_Vec3Numeric>::make(self, 2));
}

void Vec3Util::fix(const sp<Vec3>& self)
{
    ensureImpl(self)->fix();
}

sp<Vec3> Vec3Util::cross(const sp<Vec3>& self, const sp<Vec3>& other)
{
    return sp<Vec3Cross>::make(self, other);
}

sp<Vec3> Vec3Util::cross(const sp<Vec3>& self, const V3& other)
{
    return sp<Vec3Cross>::make(self, sp<Vec3::Const>::make(other));
}

sp<Vec3> Vec3Util::normalize(const sp<Vec3>& self)
{
    return sp<Vec3Normalize>::make(self);
}

sp<Vec3Impl> Vec3Util::ensureImpl(const sp<Vec3>& self)
{
    const sp<Vec3Impl> impl = self.as<Vec3Impl>();
    DCHECK(impl, "This Vec3 object is not a Vec3Impl instance");
    return impl;
}

template<> ARK_API const sp<Vec3> Null::ptr()
{
    return Ark::instance().obtain<Vec3Impl>();
}

}
