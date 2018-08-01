#include "graphics/util/vec3_util.h"

#include "core/ark.h"
#include "core/impl/variable/variable_wrapper.h"

#include "graphics/impl/vec/vec3_impl.h"
#include "graphics/impl/vec/vec_add.h"
#include "graphics/impl/vec/vec_div.h"
#include "graphics/impl/vec/vec_mul.h"
#include "graphics/impl/vec/vec_neg.h"
#include "graphics/impl/vec/vec_sub.h"
#include "graphics/impl/vec/vv2_with_transform.h"

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

}

sp<Vec3> Vec3Util::create(const sp<Numeric>& x, const sp<Numeric>& y, const sp<Numeric>& z)
{
    return sp<Vec3Impl>::make(x, y, z);
}

sp<Vec3> Vec3Util::create(float x, float y, float z)
{
    return sp<Vec3Impl>::make(x, y, z);
}

sp<Vec3> Vec3Util::add(const sp<Vec3>& self, const sp<Vec3>& rvalue)
{
    return sp<VecAdd<V3>>::make(self, rvalue);
}

sp<Vec3> Vec3Util::sub(const sp<Vec3>& self, const sp<Vec3>& rvalue)
{
    return sp<VecSub<V3>>::make(self, rvalue);
}

sp<Vec3> Vec3Util::mul(const sp<Vec3>& self, const sp<Vec3>& rvalue)
{
    return sp<VecMul<V3>>::make(self, rvalue);
}

sp<Vec3> Vec3Util::truediv(const sp<Vec3>& self, const sp<Vec3>& rvalue)
{
    return sp<VecDiv<V3>>::make(self, rvalue);
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

sp<Vec3> Vec3Util::transform(const sp<Vec3>& self, const sp<Transform>& transform, const sp<Vec3>& org)
{
    FATAL("Unimplemented");
    return nullptr;
}

V3 Vec3Util::val(const sp<Vec3>& self)
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

float Vec3Util::y(const sp<Vec3>& self)
{
    const sp<Vec3Impl> impl = self.as<Vec3Impl>();
    return impl ? impl->y()->val() : self->val().y();
}

void Vec3Util::setY(const sp<Vec3>& self, float y)
{
    ensureImpl(self)->y()->set(y);
}

sp<Numeric> Vec3Util::vx(const sp<Vec3>& self)
{
    const sp<Vec3Impl> impl = self.as<Vec3Impl>();
    return impl ? static_cast<sp<Numeric>>(impl->x()) : static_cast<sp<Numeric>>(sp<_Vec3Numeric>::make(self, 0));
}

void Vec3Util::setVx(const sp<Vec3>& self, const sp<Numeric>& x)
{
    ensureImpl(self)->x()->set(x);
}

sp<Numeric> Vec3Util::vy(const sp<Vec3>& self)
{
    const sp<Vec3Impl> impl = self.as<Vec3Impl>();
    return impl ? static_cast<sp<Numeric>>(impl->y()) : static_cast<sp<Numeric>>(sp<_Vec3Numeric>::make(self, 1));
}

void Vec3Util::setVy(const sp<Vec3>& self, const sp<Numeric>& y)
{
    ensureImpl(self)->y()->set(y);
}

void Vec3Util::fix(const sp<Vec3>& self)
{
    ensureImpl(self)->fix();
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
