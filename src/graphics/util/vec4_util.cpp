#include "graphics/util/vec4_util.h"

#include "core/ark.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/impl/variable/variable_op2.h"
#include "core/util/operators.h"

#include "graphics/impl/vec/vec4_impl.h"
#include "graphics/impl/vec/vec_neg.h"

namespace ark {

namespace {

class _Vec4Numeric : public Numeric {
public:
    _Vec4Numeric(const sp<Vec4>& delegate, int32_t dim)
        : _delegate(delegate), _dim(dim) {
    }

    virtual float val() override {
        return _delegate->val()[_dim];
    }

private:
    sp<Vec4> _delegate;
    int32_t _dim;
};

}

sp<Vec4> Vec4Util::create(const sp<Numeric>& x, const sp<Numeric>& y, const sp<Numeric>& z, const sp<Numeric>& w)
{
    return sp<Vec4Impl>::make(x, y, z, w);
}

sp<Vec4> Vec4Util::create(float x, float y, float z, float w)
{
    return sp<Vec4Impl>::make(x, y, z, w);
}

sp<Vec4> Vec4Util::add(const sp<Vec4>& lvalue, const sp<Vec4>& rvalue)
{
    return sp<VariableOP2<V4, Operators::Add<V4>, sp<Vec4>, sp<Vec4>>>::make(lvalue, rvalue);
}

sp<Vec4> Vec4Util::sub(const sp<Vec4>& lvalue, const sp<Vec4>& rvalue)
{
    return sp<VariableOP2<V4, Operators::Sub<V4>, sp<Vec4>, sp<Vec4>>>::make(lvalue, rvalue);
}

sp<Vec4> Vec4Util::mul(const sp<Vec4>& lvalue, const sp<Vec4>& rvalue)
{
    return sp<VariableOP2<V4, Operators::Mul<V4>, sp<Vec4>, sp<Vec4>>>::make(lvalue, rvalue);
}

sp<Vec4> Vec4Util::truediv(const sp<Vec4>& lvalue, const sp<Vec4>& rvalue)
{
    return sp<VariableOP2<V4, Operators::Div<V4>, sp<Vec4>, sp<Vec4>>>::make(lvalue, rvalue);
}

sp<Vec4> Vec4Util::floordiv(const sp<Vec4>& self, const sp<Vec4>& rvalue)
{
    FATAL("Unimplemented");
    return nullptr;
}

sp<Vec4> Vec4Util::negative(const sp<Vec4>& self)
{
    return sp<VecNeg<V4>>::make(self);
}

sp<Vec4> Vec4Util::transform(const sp<Vec4>& self, const sp<Transform>& transform, const sp<Vec4>& org)
{
    FATAL("Unimplemented");
    return nullptr;
}

V3 Vec4Util::xyz(const sp<Vec4>& self)
{
    return self->val();
}

V2 Vec4Util::xy(const sp<Vec4>& self)
{
    return self->val();
}

void Vec4Util::setXy(const sp<Vec4>& self, const V2& xy)
{
    const sp<Vec4Impl>& impl = ensureImpl(self);
    impl->x()->set(xy.x());
    impl->y()->set(xy.y());
}

float Vec4Util::x(const sp<Vec4>& self)
{
    const sp<Vec4Impl> impl = self.as<Vec4Impl>();
    return impl ? impl->x()->val() : self->val().x();
}

void Vec4Util::setX(const sp<Vec4>& self, float x)
{
    ensureImpl(self)->x()->set(x);
}

float Vec4Util::y(const sp<Vec4>& self)
{
    const sp<Vec4Impl> impl = self.as<Vec4Impl>();
    return impl ? impl->y()->val() : self->val().y();
}

void Vec4Util::setY(const sp<Vec4>& self, float y)
{
    ensureImpl(self)->y()->set(y);
}

sp<Numeric> Vec4Util::vx(const sp<Vec4>& self)
{
    const sp<Vec4Impl> impl = self.as<Vec4Impl>();
    return impl ? static_cast<sp<Numeric>>(impl->x()) : static_cast<sp<Numeric>>(sp<_Vec4Numeric>::make(self, 0));
}

void Vec4Util::setVx(const sp<Vec4>& self, const sp<Numeric>& x)
{
    ensureImpl(self)->x()->set(x);
}

sp<Numeric> Vec4Util::vy(const sp<Vec4>& self)
{
    const sp<Vec4Impl> impl = self.as<Vec4Impl>();
    return impl ? static_cast<sp<Numeric>>(impl->y()) : static_cast<sp<Numeric>>(sp<_Vec4Numeric>::make(self, 1));
}

void Vec4Util::setVy(const sp<Vec4>& self, const sp<Numeric>& y)
{
    ensureImpl(self)->y()->set(y);
}

void Vec4Util::fix(const sp<Vec4>& self)
{
    ensureImpl(self)->fix();
}

sp<Vec4Impl> Vec4Util::ensureImpl(const sp<Vec4>& self)
{
    const sp<Vec4Impl> impl = self.as<Vec4Impl>();
    DCHECK(impl, "This Vec4 object is not a Vec4Impl instance");
    return impl;
}

template<> ARK_API const sp<Vec4> Null::ptr()
{
    return Ark::instance().obtain<Vec4Impl>();
}

}