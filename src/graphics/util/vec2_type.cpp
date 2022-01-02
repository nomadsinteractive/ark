#include "graphics/util/vec2_type.h"

#include <algorithm>

#include "core/ark.h"
#include "core/base/clock.h"
#include "core/base/observer.h"
#include "core/inf/holder.h"
#include "core/impl/numeric/stalker.h"
#include "core/impl/variable/integral.h"
#include "core/impl/variable/integral_with_resistance.h"
#include "core/impl/variable/variable_op1.h"
#include "core/impl/variable/variable_op2.h"
#include "core/impl/variable/variable_ternary.h"
#include "core/util/holder_util.h"
#include "core/util/numeric_type.h"
#include "core/util/operators.h"
#include "core/util/variable_util.h"

#include "graphics/impl/vec/vec2_impl.h"
#include "graphics/impl/vec/vec_neg.h"
#include "graphics/impl/vec/vec2_with_transform.h"

namespace ark {

namespace {

class Vec2Numeric : public Numeric, public Holder, Implements<Vec2Numeric, Numeric, Holder> {
public:
    Vec2Numeric(const sp<Vec2>& delegate, int32_t dim)
        : _delegate(delegate), _dim(dim) {
    }

    virtual float val() override {
        return _delegate->val()[_dim];
    }

    virtual bool update(uint64_t timestamp) override {
        return _delegate->update(timestamp);
    }

    virtual void traverse(const Visitor& visitor) override {
        HolderUtil::visit(_delegate, visitor);
    }

private:
    sp<Vec2> _delegate;
    int32_t _dim;

};

class Vec2Normalize : public Vec2, public Holder, Implements<Vec2Normalize, Vec2, Holder> {
public:
    Vec2Normalize(const sp<Vec2>& value)
        : _delegate(value) {
    }

    virtual V2 val() override {
        V2 val = _delegate->val();
        float hypot = std::max(Math::hypot(val.x(), val.y()), 0.000001f);
        return val / hypot;
    }

    virtual bool update(uint64_t timestamp) override {
        return _delegate->update(timestamp);
    }

    virtual void traverse(const Visitor& visitor) override {
        HolderUtil::visit(_delegate, visitor);
    }

private:
    sp<Vec2> _delegate;

};

class Vec2Fence : public Vec2, public Holder, Implements<Vec2Fence, Vec2, Holder> {
public:
    Vec2Fence(sp<Vec2> delegate, sp<Vec3> plane, sp<Observer> observer)
        : _delegate(std::move(delegate)), _plane(std::move(plane)), _observer(std::move(observer)), _distance(getPlaneDistance(_delegate->val())) {
    }

    virtual V2 val() override {
        V2 v = _delegate->val();
        float distance = getPlaneDistance(v);
        if(!Math::signEquals(_distance, distance)) {
            _observer->update();
            _distance = distance;
        }
        return v;
    }

    virtual bool update(uint64_t timestamp) override {
        return VariableUtil::update(timestamp, _delegate, _plane);
    }

    virtual void traverse(const Visitor& visitor) override {
        HolderUtil::visit(_delegate, visitor);
        HolderUtil::visit(_plane, visitor);
        _observer->traverse(visitor);
    }

private:
    float getPlaneDistance(const V2& pos) const {
        return V3(pos.x(), pos.y(), 1.0f).dot(_plane->val());
    }

private:
    sp<Vec2> _delegate;
    sp<Vec3> _plane;
    sp<Observer> _observer;

    float _distance;
};

}

static float _atan2(const V2& val)
{
    return Math::atan2(val.y(), val.x());
}

sp<Vec2> Vec2Type::create(const sp<Numeric>& x, const sp<Numeric>& y)
{
    return sp<Vec2Impl>::make(x, y);
}

sp<Vec2> Vec2Type::create(float x, float y)
{
    return sp<Vec2Impl>::make(x, y);
}

sp<Vec2> Vec2Type::add(const sp<Vec2>& lvalue, const sp<Vec2>& rvalue)
{
    return sp<VariableOP2<sp<Vec2>, sp<Vec2>, Operators::Add<V2>>>::make(lvalue, rvalue);
}

sp<Vec2> Vec2Type::sub(const sp<Vec2>& lvalue, const sp<Vec2>& rvalue)
{
    return sp<VariableOP2<sp<Vec2>, sp<Vec2>, Operators::Sub<V2>>>::make(lvalue, rvalue);
}

sp<Vec2> Vec2Type::mul(const sp<Vec2>& lvalue, const sp<Vec2>& rvalue)
{
    return sp<VariableOP2<sp<Vec2>, sp<Vec2>, Operators::Mul<V2>>>::make(lvalue, rvalue);
}

sp<Vec2> Vec2Type::mul(const sp<Vec2>& lvalue, float rvalue)
{
    return sp<VariableOP2<sp<Vec2>, float, Operators::Mul<V2, float>>>::make(lvalue, rvalue);
}

sp<Vec2> Vec2Type::mul(float lvalue, const sp<Vec2>& rvalue)
{
    return sp<VariableOP2<float, sp<Vec2>, Operators::Mul<float, V2>>>::make(lvalue, rvalue);
}

sp<Vec2> Vec2Type::mul(const sp<Vec2>& lvalue, sp<Numeric>& rvalue)
{
    return sp<VariableOP2<sp<Vec2>, sp<Numeric>, Operators::Mul<V2, float>>>::make(lvalue, rvalue);
}

sp<Vec2> Vec2Type::truediv(const sp<Vec2>& lvalue, const sp<Vec2>& rvalue)
{
    return sp<VariableOP2<sp<Vec2>, sp<Vec2>, Operators::Div<V2>>>::make(lvalue, rvalue);
}

sp<Vec2> Vec2Type::truediv(const sp<Vec2>& lvalue, const sp<Numeric>& rvalue)
{
    return sp<VariableOP2<sp<Vec2>, sp<Numeric>, Operators::Div<V2, float>>>::make(lvalue, rvalue);
}

sp<Vec2> Vec2Type::truediv(const sp<Vec2>& lvalue, float rvalue)
{
    return sp<VariableOP2<sp<Vec2>, float, Operators::Div<V2, float>>>::make(lvalue, rvalue);
}

sp<Vec2> Vec2Type::negative(const sp<Vec2>& self)
{
    return sp<VecNeg<V2>>::make(self);
}

sp<Vec2> Vec2Type::transform(const sp<Vec2>& self, const sp<Transform>& transform)
{
    return sp<Vec2WithTransform>::make(self, transform);
}

sp<Vec2> Vec2Type::normalize(const sp<Vec2>& self)
{
    return sp<Vec2Normalize>::make(self);
}

sp<Vec2> Vec2Type::integral(const sp<Vec2>& self, const sp<Numeric>& t)
{
    sp<Numeric> duration = t ? t : Ark::instance().clock()->duration();
    return sp<Integral<V2>>::make(self, std::move(duration));
}

sp<Vec2> Vec2Type::integralWithResistance(const sp<Vec2>& self, const V2& v0, const sp<Numeric>& cd, const sp<Numeric>& t)
{
    return sp<IntegralWithResistance<V2>>::make(v0, self, cd, t ? t : Ark::instance().clock()->duration());
}

void Vec2Type::set(const sp<VariableWrapper<V2>>& self, const V2& val)
{
    self->set(val);
}

void Vec2Type::set(const sp<VariableWrapper<V2>>& self, const sp<Vec2>& val)
{
    self->set(val);
}

void Vec2Type::set(const sp<Vec2>& self, const V2 val)
{
    ensureImpl(self)->set(val);
}

V2 Vec2Type::val(const sp<Vec2>& self)
{
    return self->val();
}

V2 Vec2Type::xy(const sp<Vec2>& self)
{
    return self->val();
}

void Vec2Type::setXy(const sp<Vec2>& self, const V2& xy)
{
    ensureImpl(self)->set(xy);
}

float Vec2Type::x(const sp<Vec2>& self)
{
    const sp<Vec2Impl> impl = self.as<Vec2Impl>();
    return impl ? impl->x()->val() : self->val().x();
}

void Vec2Type::setX(const sp<Vec2>& self, float x)
{
    ensureImpl(self)->x()->set(x);
}

void Vec2Type::setX(const sp<Vec2>& self, const sp<Numeric>& x)
{
    ensureImpl(self)->x()->set(x);
}

float Vec2Type::y(const sp<Vec2>& self)
{
    const sp<Vec2Impl> impl = self.as<Vec2Impl>();
    return impl ? impl->y()->val() : self->val().y();
}

void Vec2Type::setY(const sp<Vec2>& self, float y)
{
    ensureImpl(self)->y()->set(y);
}

void Vec2Type::setY(const sp<Vec2>& self, const sp<Numeric>& y)
{
    ensureImpl(self)->y()->set(y);
}

sp<Numeric> Vec2Type::vx(const sp<Vec2>& self)
{
    const sp<Vec2Impl> impl = self.as<Vec2Impl>();
    return impl ? static_cast<sp<Numeric>>(impl->x()) : static_cast<sp<Numeric>>(sp<Vec2Numeric>::make(self, 0));
}

void Vec2Type::setVx(const sp<Vec2>& self, const sp<Numeric>& x)
{
    ensureImpl(self)->x()->set(x);
}

sp<Numeric> Vec2Type::vy(const sp<Vec2>& self)
{
    const sp<Vec2Impl> impl = self.as<Vec2Impl>();
    return impl ? static_cast<sp<Numeric>>(impl->y()) : static_cast<sp<Numeric>>(sp<Vec2Numeric>::make(self, 1));
}

void Vec2Type::setVy(const sp<Vec2>& self, const sp<Numeric>& y)
{
    ensureImpl(self)->y()->set(y);
}

void Vec2Type::fix(const sp<Vec2>& self)
{
    sp<VariableWrapper<V2>> wrapper = self.as<VariableWrapper<V2>>();
    if(wrapper)
    {
        wrapper->fix();
        return;
    }
    sp<Vec2Impl> impl = self.as<Vec2Impl>();
    DCHECK(impl, "Object is not an instance of neither VariableWrapper<V2> or Vec2Impl");
    ensureImpl(self)->fix();
}

sp<Vec2> Vec2Type::freeze(const sp<Vec2>& self)
{
    return sp<Vec2::Const>::make(self->val());
}

sp<Vec2> Vec2Type::wrap(const sp<Vec2>& self)
{
    return sp<VariableWrapper<V2>>::make(self);
}

sp<Vec2> Vec2Type::synchronize(const sp<Vec2>& self, const sp<Boolean>& disposed)
{
    return Ark::instance().applicationContext()->synchronize(self, disposed);
}

sp<Vec2> Vec2Type::modFloor(const sp<Vec2>& self, const sp<Numeric>& mod)
{
    return create(NumericType::modFloor(vx(self), mod), NumericType::modFloor(vy(self), mod));
}

sp<Vec2> Vec2Type::attract(const sp<Vec2>& self, const V2& s0, float duration, const sp<Numeric>& t)
{
    sp<Numeric> ts = t ? t : Ark::instance().clock()->duration();
    return sp<Vec2Impl>::make(sp<Stalker>::make(ts, vx(self), s0.x(), duration), sp<Stalker>::make(ts, vy(self), s0.y(), duration));
}

sp<Vec2> Vec2Type::fence(const sp<Vec2>& self, const sp<Vec3>& plane, const sp<Observer>& observer)
{
    return sp<Vec2Fence>::make(self, plane, observer);
}

sp<Vec2> Vec2Type::ifElse(const sp<Vec2>& self, const sp<Boolean>& condition, const sp<Vec2>& negative)
{
    return sp<VariableTernary<V2>>::make(condition, self, negative);
}

sp<Numeric> Vec2Type::atan2(const sp<Vec2>& self)
{
    return sp<VariableOP1<float, V2>>::make(_atan2, self);
}

sp<Vec2> Vec2Type::delegate(const sp<Vec2>& self)
{
    sp<VariableWrapper<V2>> wrapper = self.as<VariableWrapper<V2>>();
    DWARN(wrapper, "Non-Vec2Wrapper instance has no delegate attribute. This should be an error unless you're inspecting it.");
    return wrapper ? wrapper->delegate() : nullptr;
}

void Vec2Type::setDelegate(const sp<Vec2>& self, const sp<Vec2>& delegate)
{
    sp<VariableWrapper<V2>> wrapper = self.as<VariableWrapper<V2>>();
    DCHECK(wrapper, "Must be an Vec2Wrapper instance to set its delegate attribute");
    wrapper->set(delegate);
}

sp<Vec2Impl> Vec2Type::ensureImpl(const sp<Vec2>& self)
{
    const sp<Vec2Impl> impl = self.as<Vec2Impl>();
    DCHECK(impl, "This Vec2 object is not a Vec2Impl instance");
    return impl;
}

template<> ARK_API sp<Vec2> Null::ptr()
{
    return sp<Vec2Impl>::make();
}

}
