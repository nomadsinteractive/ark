#include "graphics/util/vec2_util.h"

#include "core/ark.h"
#include "core/impl/variable/variable_wrapper.h"

#include "graphics/impl/vec/vec2_impl.h"
#include "graphics/impl/vec/vec_add.h"
#include "graphics/impl/vec/vec_div.h"
#include "graphics/impl/vec/vec_mul.h"
#include "graphics/impl/vec/vec_neg.h"
#include "graphics/impl/vec/vec_sub.h"
#include "graphics/impl/vec/vv2_with_transform.h"

namespace ark {

namespace {

class _Vec2Numeric : public Numeric {
public:
    _Vec2Numeric(const sp<Vec2>& delegate, int32_t dim)
        : _delegate(delegate), _dim(dim) {
    }

    virtual float val() override {
        return _delegate->val()[_dim];
    }

private:
    sp<Vec2> _delegate;
    int32_t _dim;
};

}

sp<Vec2> Vec2Util::create(const sp<Numeric>& x, const sp<Numeric>& y)
{
    return sp<Vec2Impl>::make(x, y);
}

sp<Vec2> Vec2Util::create(float x, float y)
{
    return sp<Vec2Impl>::make(x, y);
}

sp<Vec2> Vec2Util::add(const sp<Vec2>& self, const sp<Vec2>& rvalue)
{
    return sp<VecAdd<V2>>::make(self, rvalue);
}

sp<Vec2> Vec2Util::sub(const sp<Vec2>& self, const sp<Vec2>& rvalue)
{
    return sp<VecSub<V2>>::make(self, rvalue);
}

sp<Vec2> Vec2Util::mul(const sp<Vec2>& self, const sp<Vec2>& rvalue)
{
    return sp<VecMul<V2>>::make(self, rvalue);
}

sp<Vec2> Vec2Util::truediv(const sp<Vec2>& self, const sp<Vec2>& rvalue)
{
    return sp<VecDiv<V2>>::make(self, rvalue);
}

sp<Vec2> Vec2Util::floordiv(const sp<Vec2>& self, const sp<Vec2>& rvalue)
{
    FATAL("Unimplemented");
    return nullptr;
}

sp<Vec2> Vec2Util::negative(const sp<Vec2>& self)
{
    return sp<VecNeg<V2>>::make(self);
}

sp<Vec2> Vec2Util::transform(const sp<Vec2>& self, const sp<Transform>& transform, const sp<Vec2>& org)
{
    return sp<Vec2WithTransform>::make(self, org, transform);
}

V2 Vec2Util::val(const sp<Vec2>& self)
{
    return self->val();
}

V2 Vec2Util::xy(const sp<Vec2>& self)
{
    return self->val();
}

void Vec2Util::setXy(const sp<Vec2>& self, const V2& xy)
{
    ensureImpl(self)->set(xy);
}

float Vec2Util::x(const sp<Vec2>& self)
{
    const sp<Vec2Impl> impl = self.as<Vec2Impl>();
    return impl ? impl->x()->val() : self->val().x();
}

void Vec2Util::setX(const sp<Vec2>& self, float x)
{
    ensureImpl(self)->x()->set(x);
}

float Vec2Util::y(const sp<Vec2>& self)
{
    const sp<Vec2Impl> impl = self.as<Vec2Impl>();
    return impl ? impl->y()->val() : self->val().y();
}

void Vec2Util::setY(const sp<Vec2>& self, float y)
{
    ensureImpl(self)->y()->set(y);
}

sp<Numeric> Vec2Util::vx(const sp<Vec2>& self)
{
    const sp<Vec2Impl> impl = self.as<Vec2Impl>();
    return impl ? static_cast<sp<Numeric>>(impl->x()) : static_cast<sp<Numeric>>(sp<_Vec2Numeric>::make(self, 0));
}

void Vec2Util::setVx(const sp<Vec2>& self, const sp<Numeric>& x)
{
    ensureImpl(self)->x()->set(x);
}

sp<Numeric> Vec2Util::vy(const sp<Vec2>& self)
{
    const sp<Vec2Impl> impl = self.as<Vec2Impl>();
    return impl ? static_cast<sp<Numeric>>(impl->y()) : static_cast<sp<Numeric>>(sp<_Vec2Numeric>::make(self, 1));
}

void Vec2Util::setVy(const sp<Vec2>& self, const sp<Numeric>& y)
{
    ensureImpl(self)->y()->set(y);
}

void Vec2Util::fix(const sp<Vec2>& self)
{
    ensureImpl(self)->fix();
}

sp<Vec2Impl> Vec2Util::ensureImpl(const sp<Vec2>& self)
{
    const sp<Vec2Impl> impl = self.as<Vec2Impl>();
    DCHECK(impl, "This Vec2 object is not a Vec2Impl instance");
    return impl;
}

template<> ARK_API const sp<Vec2> Null::ptr()
{
    return Ark::instance().obtain<Vec2Impl>();
}

}