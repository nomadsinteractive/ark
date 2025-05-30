#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/clock.h"
#include "core/impl/variable/variable_dyed.h"
#include "core/impl/variable/integral.h"
#include "core/impl/variable/lerp.h"
#include "core/impl/variable/second_order_dynamics.h"
#include "core/impl/variable/variable_op1.h"
#include "core/impl/variable/variable_op2.h"
#include "core/impl/variable/variable_ternary.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/inf/variable.h"
#include "core/types/optional.h"
#include "core/types/shared_ptr.h"
#include "core/util/operators.h"

#include "graphics/forwarding.h"
#include "graphics/components/size.h"
#include "graphics/impl/vec/vec_subscribed.h"

#include "app/base/application_context.h"
#include "core/impl/variable/variable_tracking.h"

namespace ark {

template<typename T, typename IMPL> class VecType {
public:
    typedef Variable<T> VarType;

    enum {
        DIMENSION = sizeof(T) / sizeof(float)
    };

    static T val(const sp<VarType>& self) {
        self->update(0);
        return self->val();
    }

    static sp<VarType> add(sp<VarType> a, const T b) {
        return sp<VarType>::template make<VariableOP2<sp<VarType>, T, Operators::Add<T>>>(std::move(a), b);
    }

    static sp<VarType> add(sp<VarType> a, sp<VarType> b) {
        return sp<VarType>::template make<VariableOP2<sp<VarType>, sp<VarType>, Operators::Add<T>>>(std::move(a), std::move(b));
    }

    static sp<VarType> sub(sp<VarType> a, const T b) {
        return sp<VarType>::template make<VariableOP2<sp<VarType>, T, Operators::Sub<T>>>(std::move(a), b);
    }

    static sp<VarType> sub(sp<VarType> a, sp<VarType> b) {
        return sp<VarType>::template make<VariableOP2<sp<VarType>, sp<VarType>, Operators::Sub<T>>>(std::move(a), std::move(b));
    }

    static sp<VarType> mul(sp<VarType> a, const T b) {
        return sp<VarType>::template make<VariableOP2<sp<VarType>, T, Operators::Mul<T>>>(std::move(a), b);
    }

    static sp<VarType> mul(sp<VarType> a, sp<VarType> b) {
        return sp<VarType>::template make<VariableOP2<sp<VarType>, sp<VarType>, Operators::Mul<T>>>(std::move(a), std::move(b));
    }

    static sp<VarType> mul(sp<VarType> a, const float b) {
        return sp<VarType>::template make<VariableOP2<sp<VarType>, float, Operators::Mul<T, float>>>(std::move(a), b);
    }

    static sp<VarType> mul(sp<Numeric> a, sp<VarType> b) {
        return mul(std::move(b), std::move(a));
    }

    static sp<VarType> mul(const float a, sp<VarType> b) {
        return mul(std::move(b), a);
    }

    static sp<VarType> mul(sp<VarType> a, sp<Numeric> b) {
        return sp<VarType>::template make<VariableOP2<sp<VarType>, sp<Numeric>, Operators::Mul<T, float>>>(std::move(a), std::move(b));
    }

    static sp<VarType> truediv(sp<VarType> a, sp<VarType> b) {
        return sp<VarType>::template make<VariableOP2<sp<VarType>, sp<VarType>, Operators::Div<T>>>(std::move(a), std::move(b));
    }

    static sp<VarType> truediv(sp<VarType> a, const T b) {
        return sp<VarType>::template make<VariableOP2<sp<VarType>, T, Operators::Div<T>>>(std::move(a), b);
    }

    static sp<VarType> truediv(sp<VarType> a, sp<Numeric> b) {
        return sp<VarType>::template make<VariableOP2<sp<VarType>, sp<Numeric>, Operators::Div<T, float>>>(std::move(a), std::move(b));
    }

    static sp<VarType> floordiv(sp<VarType> a, const float b) {
        return sp<VarType>::template make<VariableOP2<sp<VarType>, T, Operators::FloorDiv<T>>>(std::move(a), T(b));
    }

    static sp<VarType> floordiv(sp<VarType> a, sp<Numeric> b) {
        return sp<VarType>::template make<VariableOP2<sp<VarType>, sp<VarType>, Operators::FloorDiv<T>>>(std::move(a), sp<IMPL>::make(std::move(b)));
    }

    static sp<VarType> floordiv(sp<VarType> a, sp<VarType> b) {
        return sp<VarType>::template make<VariableOP2<sp<VarType>, sp<VarType>, Operators::FloorDiv<T>>>(std::move(a), std::move(b));
    }

    static sp<VarType> truediv(sp<VarType> a, float b) {
        return sp<VarType>::template make<VariableOP2<sp<VarType>, float, Operators::Div<T, float>>>(std::move(a), b);
    }

    static sp<VarType> negative(sp<VarType> self) {
        return sp<VarType>::template make<VariableOP1<T>>(Operators::Neg<T>(), std::move(self));
    }

    static sp<VarType> absolute(sp<VarType> self) {
        return sp<VarType>::template make<VariableOP1<T>>(Operators::Abs<T>(), std::move(self));
    }

    static sp<VarType> normalize(sp<VarType> self) {
        return sp<VarType>::template make<VariableOP1<T>>(Operators::Normalize<T>(), std::move(self));
    }

    static sp<VarType> integral(sp<VarType> self, sp<Numeric> t) {
        sp<Numeric> duration = t ? std::move(t) : Ark::instance().appClock()->duration();
        return sp<VarType>::template make<Integral<T>>(std::move(self), std::move(duration));
    }

    static sp<VarType> integralS2(sp<VarType> self, const T& s0, const Optional<T>& s1, sp<Numeric> t) {
        sp<Numeric> duration = t ? std::move(t) : Ark::instance().appClock()->duration();
        return sp<VarType>::template make<IntegralS2<T>>(std::move(self), std::move(duration), s0, s1 ? s1.value() : s0);
    }

    static sp<Numeric> distance(sp<VarType> self, sp<VarType> other) {
        return Math::distance(std::move(self), std::move(other));
    }

    static sp<Numeric> distance2(sp<VarType> self, sp<VarType> other) {
        return sp<Numeric>::make<VariableOP2<sp<VarType>, sp<VarType>, Operators::Distance2<T>>>(std::move(self), std::move(other));
    }

    static sp<Numeric> hypot(sp<VarType> self) {
        return sp<Numeric>::make<VariableOP1<float, T>>(Operators::Hypot<T>(), std::move(self));
    }

    static sp<Numeric> hypot2(sp<VarType> self) {
        return sp<Numeric>::make<VariableOP1<float, T>>(Operators::Hypot2<T>(), std::move(self));
    }

    static void set(const sp<VariableWrapper<T>>& self, sp<VarType> val) {
        self->set(std::move(val));
    }

    static void set(const sp<VariableWrapper<T>>& self, const T& val) {
        self->set(val);
    }

    static void set(const sp<VarType>& self, const T& val) {
        ensureImpl(self)->set(val);
    }

    static sp<Numeric> x(sp<VarType> self) {
        return sp<Numeric>::make<VariableOP1<float, T>>(Operators::Subscript<T, float>(0), std::move(self));
    }

    static void setX(const sp<VarType>& self, float x) {
        ensureImpl(self)->x()->set(x);
    }

    static void setX(const sp<VarType>& self, sp<Numeric> x) {
        ensureImpl(self)->x()->set(std::move(x));
    }

    static sp<Numeric> y(const sp<VarType>& self) {
        return sp<Numeric>::make<VariableOP1<float, T>>(Operators::Subscript<T, float>(1), std::move(self));
    }

    static void setY(const sp<VarType>& self, float y) {
        ensureImpl(self)->y()->set(y);
    }

    static void setY(const sp<VarType>& self, sp<Numeric> y) {
        ensureImpl(self)->y()->set(std::move(y));
    }

    static sp<Numeric> z(const sp<VarType>& self) {
        if constexpr(DIMENSION > 2)
            return sp<Numeric>::make<VariableOP1<float, T>>(Operators::Subscript<T, float>(2), std::move(self));
        FATAL("You shouldn't be here");
        return nullptr;
    }

    template<typename U> static void setZ(const sp<VarType>& self, U z) {
        if constexpr(DIMENSION > 2)
            ensureImpl(self)->z()->set(std::move(z));
        else
            FATAL("You shouldn't be here");
    }

    static size_t len(const sp<VarType>& /*self*/) {
        return DIMENSION;
    }

    static Optional<float> getItem(const sp<VarType>& self, const ptrdiff_t idx) {
        return (idx >= 0 && idx < DIMENSION) ? Optional<float>(self->val()[idx]) : Optional<float>();
    }

    static sp<Vec2> xy(sp<VarType> self) {
        if constexpr(std::is_same_v<V2, T>)
            return self;
        return sp<Vec2>::make<VecSubscribed<V2, T>>(std::move(self), std::array<uint8_t, 2>{0, 1});
    }

    static sp<Vec2> xz(const sp<VarType>& self) {
        CHECK(2 < DIMENSION, "Index z(3) out of bounds");
        return sp<Vec2>::make<VecSubscribed<V2, T>>(std::move(self), std::array<uint8_t, 2>{0, 2});
    }

    static sp<Vec2> yx(const sp<VarType>& self) {
        return sp<Vec2>::make<VecSubscribed<V2, T>>(std::move(self), std::array<uint8_t, 2>{1, 0});
    }

    static sp<Vec2> yz(const sp<VarType>& self) {
        CHECK(2 < DIMENSION, "Index z(3) out of bounds");
        return sp<Vec2>::make<VecSubscribed<V2, T>>(std::move(self), std::array<uint8_t, 2>{1, 2});
    }

    static sp<Vec2> zx(const sp<VarType>& self) {
        CHECK(2 < DIMENSION, "Index z(3) out of bounds");
        return sp<Vec2>::make<VecSubscribed<V2, T>>(std::move(self), std::array<uint8_t, 2>{2, 0});
    }

    static sp<Vec2> zy(const sp<VarType>& self) {
        CHECK(2 < DIMENSION, "Index z(3) out of bounds");
        return sp<Vec2>::make<VecSubscribed<V2, T>>(std::move(self), std::array<uint8_t, 2>{2, 1});
    }

    static sp<Vec3> xyz(sp<VarType> self) {
        CHECK(2 < DIMENSION, "Index z(3) out of bounds");
        if constexpr(std::is_same_v<V3, T>)
            return self;
        return sp<Vec3>::make<VecSubscribed<V3, T>>(std::move(self), std::array<uint8_t, 3>{0, 1, 2});
    }

    static sp<VarType> freeze(const sp<VarType>& self) {
        return self ? sp<VarType>::template make<typename VarType::Const>(val(self)) : nullptr;
    }

    static sp<VarType> wrap(sp<VarType> self) {
        return sp<VarType>::template make<VariableWrapper<T>>(std::move(self));
    }

    static sp<VarType> synchronize(sp<VarType> self, sp<Boolean> canceled) {
        return Ark::instance().renderController()->synchronize(std::move(self), std::move(canceled));
    }

    static sp<VarType> floor(sp<VarType> self, sp<Numeric> mod) {
        if(mod)
            return sp<VarType>::template make<VariableOP2<sp<VarType>, sp<VarType>, Operators::ModFloor<T>>>(std::move(self), sp<IMPL>::make(std::move(mod)));
        return sp<VarType>::template make<VariableOP1<T>>(Operators::Transform<T, typename Operators::Floor<float>, DIMENSION>(Operators::Floor<float>()), std::move(self));
    }

    static sp<VarType> floor(sp<VarType> self, sp<VarType> mod) {
        if(mod)
            return sp<VarType>::template make<VariableOP2<sp<VarType>, sp<VarType>, Operators::ModFloor<T>>>(std::move(self), std::move(mod));
        return sp<VarType>::template make<VariableOP1<T>>(Operators::Transform<T, typename Operators::Floor<float>, DIMENSION>(Operators::Floor<float>()), std::move(self));
    }

    static sp<VarType> ceil(sp<VarType> self, sp<Numeric> mod) {
        if(mod)
            return sp<VarType>::template make<VariableOP2<sp<VarType>, sp<VarType>, Operators::ModCeil<T>>>(std::move(self), sp<IMPL>::make(std::move(mod)));
        return sp<VarType>::template make<VariableOP1<T>>(Operators::Transform<T, typename Operators::Ceil<float>, DIMENSION>(Operators::Ceil<float>()), std::move(self));
    }

    static sp<VarType> ceil(sp<VarType> self, sp<VarType> mod) {
        if(mod)
            return sp<VarType>::template make<VariableOP2<sp<VarType>, sp<VarType>, Operators::ModCeil<T>>>(std::move(self), std::move(mod));
        return sp<VarType>::template make<VariableOP1<T>>(Operators::Transform<T, typename Operators::Ceil<float>, DIMENSION>(Operators::Ceil<float>()), std::move(self));
    }

    static sp<VarType> round(sp<VarType> self) {
        return sp<VarType>::template make<VariableOP1<T>>(Operators::Transform<T, typename Operators::Round<float>, DIMENSION>(Operators::Round<float>()), std::move(self));
    }

    static sp<VarType> ifElse(sp<VarType> self, sp<Boolean> condition, sp<VarType> otherwise) {
        return sp<VarType>::template make<VariableTernary<T>>(std::move(condition), std::move(self), std::move(otherwise));
    }

    static sp<VarType> wrapped(const sp<VarType>& self) {
        sp<VariableWrapper<T>> wrapper = self.template asInstance<VariableWrapper<T>>();
        CHECK_WARN(wrapper, "Non-Vec%dWrapper instance has no delegate attribute. This should be an error unless you're inspecting it.", DIMENSION);
        return wrapper ? wrapper->wrapped() : nullptr;
    }

    static sp<Size> toSize(const sp<VarType>& self) {
        return sp<Size>::make(x(self), y(self), DIMENSION > 2 ? z(self) : sp<Numeric>::make<Numeric::Const>(0.0f));
    }

    static sp<VarType> lerp(sp<VarType> self, sp<VarType> b, sp<Numeric> t) {
        if(!t)
            t = Ark::instance().appClock()->duration();
        return sp<VarType>::template make<Lerp<T, float>>(std::move(self), std::move(b), std::move(t));
    }

    static sp<VarType> track(sp<VarType> self, const T s0, const float speed, const float distance, sp<Future> future, sp<Numeric> t) {
        if(!t)
            t = Ark::instance().appClock()->duration();
        return sp<VarType>::template make<VariableTracking<T>>(std::move(self), std::move(t), s0, speed, distance, std::move(future));
    }

    static sp<VarType> sod(sp<VarType> self, const T s0, const float f, const float z, const float r, sp<Numeric> t) {
        if(!t)
            t = Ark::instance().appClock()->duration();
        return sp<VarType>::template make<SecondOrderDynamics<T>>(std::move(self), s0, std::move(t), f, z, r);
    }

    static sp<VarType> dye(sp<VarType> self, sp<Boolean> c, String message) {
        return sp<VarType>::template make<VariableDyed<T>>(std::move(self), std::move(c), std::move(message));
    }

protected:
    static sp<IMPL> toImpl(const sp<VarType>& self) {
        return self.template asInstance<IMPL>();
    }

    static sp<IMPL> ensureImpl(const sp<VarType>& self) {
        sp<IMPL> impl = self.template asInstance<IMPL>();
        CHECK(impl, "This Vec%d object is not a Vec%dImpl instance", DIMENSION, DIMENSION);
        return impl;
    }
};

}
