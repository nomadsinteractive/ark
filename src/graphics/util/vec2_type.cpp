#include "graphics/util/vec2_type.h"

#include <algorithm>

#include "core/ark.h"
#include "core/base/clock.h"
#include "core/base/observer.h"
#include "core/inf/holder.h"
#include "core/impl/variable/integral.h"
#include "core/impl/variable/integral_with_resistance.h"
#include "core/impl/variable/second_order_dynamics.h"
#include "core/impl/variable/variable_op1.h"
#include "core/impl/variable/variable_op2.h"
#include "core/impl/variable/variable_ternary.h"
#include "core/util/holder_util.h"
#include "core/util/numeric_type.h"
#include "core/util/operators.h"
#include "core/util/variable_util.h"

#include "graphics/base/size.h"
#include "graphics/impl/vec/vec2_impl.h"
#include "graphics/impl/vec/vec2_with_transform.h"

namespace ark {

namespace {

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

sp<Vec2> Vec2Type::create(sp<Numeric> x, sp<Numeric> y)
{
    return sp<Vec2Impl>::make(std::move(x), std::move(y));
}

sp<Vec2> Vec2Type::create(float x, float y)
{
    return sp<Vec2Impl>::make(x, y);
}

sp<Vec2> Vec2Type::transform(const sp<Vec2>& self, const sp<Transform>& transform)
{
    return sp<Vec2WithTransform>::make(self, transform);
}

sp<Vec3> Vec2Type::extend(sp<Vec2> self, sp<Numeric> z)
{
    return sp<VariableOP2<sp<Vec2>, sp<Numeric>, Operators::Extend<V2, float>>>::make(std::move(self), std::move(z));
}

sp<Vec4> Vec2Type::extend(sp<Vec2> self, sp<Vec2> zw)
{
    return sp<VariableOP2<sp<Vec2>, sp<Vec2>, Operators::Extend<V2, V2>>>::make(std::move(self), std::move(zw));
}

sp<Vec2> Vec2Type::fence(sp<Vec2> self, sp<Vec3> plane, sp<Observer> observer)
{
    return sp<Vec2Fence>::make(std::move(self), std::move(plane), std::move(observer));
}

sp<Numeric> Vec2Type::atan2(sp<Vec2> self)
{
    return sp<VariableOP1<float, V2>>::make(_atan2, std::move(self));
}

template<> ARK_API sp<Vec2> Null::ptr()
{
    return sp<Vec2Impl>::make();
}

}
