#ifndef ARK_GRAPHICS_UTIL_VEC_TYPE_H_
#define ARK_GRAPHICS_UTIL_VEC_TYPE_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/clock.h"
#include "core/impl/variable/integral.h"
#include "core/impl/variable/integral_with_resistance.h"
#include "core/impl/variable/interpolate.h"
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
#include "graphics/base/v4.h"
#include "graphics/base/size.h"

namespace ark {

template<typename T, typename IMPL> class VecType {
public:
    typedef Variable<T> VarType;

    enum {
        DIMENSION = sizeof(T) / sizeof(float)
    };

    static T val(const sp<VarType>& self) {
        return self->val();
    }

    static sp<VarType> add(sp<VarType> lvalue, const T& rvalue) {
        return sp<VariableOP2<sp<VarType>, T, Operators::Add<T>>>::make(std::move(lvalue), rvalue);
    }

    static sp<VarType> add(sp<VarType> lvalue, sp<VarType> rvalue) {
        return sp<VariableOP2<sp<VarType>, sp<VarType>, Operators::Add<T>>>::make(std::move(lvalue), std::move(rvalue));
    }

    static sp<VarType> sub(sp<VarType> lvalue, const T& rvalue) {
        return sp<VariableOP2<sp<VarType>, T, Operators::Sub<T>>>::make(std::move(lvalue), rvalue);
    }

    static sp<VarType> sub(sp<VarType> lvalue, sp<VarType> rvalue) {
        return sp<VariableOP2<sp<VarType>, sp<VarType>, Operators::Sub<T>>>::make(std::move(lvalue), std::move(rvalue));
    }

    static sp<VarType> mul(sp<VarType> lvalue, const T& rvalue) {
        return sp<VariableOP2<sp<VarType>, T, Operators::Mul<T>>>::make(std::move(lvalue), rvalue);
    }

    static sp<VarType> mul(sp<VarType> lvalue, sp<VarType> rvalue) {
        return sp<VariableOP2<sp<VarType>, sp<VarType>, Operators::Mul<T>>>::make(std::move(lvalue), std::move(rvalue));
    }

    static sp<VarType> mul(sp<VarType> lvalue, float rvalue) {
        return sp<VariableOP2<sp<VarType>, float, Operators::Mul<T, float>>>::make(std::move(lvalue), rvalue);
    }

    static sp<VarType> mul(sp<Numeric> lvalue, sp<VarType> rvalue) {
        return mul(std::move(rvalue), std::move(lvalue));
    }

    static sp<VarType> mul(sp<VarType> lvalue, sp<Numeric> rvalue) {
        return sp<VariableOP2<sp<VarType>, sp<Numeric>, Operators::Mul<T, float>>>::make(std::move(lvalue), std::move(rvalue));
    }

    static sp<VarType> truediv(sp<VarType> lvalue, sp<VarType> rvalue) {
        return sp<VariableOP2<sp<VarType>, sp<VarType>, Operators::Div<T>>>::make(std::move(lvalue), std::move(rvalue));
    }

    static sp<VarType> truediv(sp<VarType> lvalue, const T& rvalue) {
        return sp<VariableOP2<sp<VarType>, T, Operators::Div<T>>>::make(std::move(lvalue), std::move(rvalue));
    }

    static sp<VarType> truediv(sp<VarType> lvalue, sp<Numeric> rvalue) {
        return sp<VariableOP2<sp<VarType>, sp<Numeric>, Operators::Div<T, float>>>::make(std::move(lvalue), std::move(rvalue));
    }

    static sp<VarType> floordiv(sp<VarType> lvalue, float rvalue) {
        return sp<VariableOP2<sp<VarType>, T, Operators::FloorDiv<T>>>::make(std::move(lvalue), T(rvalue));
    }

    static sp<VarType> floordiv(sp<VarType> lvalue, sp<Numeric> rvalue) {
        return sp<VariableOP2<sp<VarType>, sp<VarType>, Operators::FloorDiv<T>>>::make(std::move(lvalue), sp<IMPL>::make(std::move(rvalue)));
    }

    static sp<VarType> floordiv(sp<VarType> lvalue, sp<VarType> rvalue) {
        return sp<VariableOP2<sp<VarType>, sp<VarType>, Operators::FloorDiv<T>>>::make(std::move(lvalue), std::move(rvalue));
    }

    static sp<VarType> truediv(sp<VarType> lvalue, float rvalue) {
        return sp<VariableOP2<sp<VarType>, float, Operators::Div<T, float>>>::make(std::move(lvalue), rvalue);
    }

    static sp<VarType> negative(sp<VarType> self) {
        return sp<VariableOP1<T>>::make(Operators::Neg<T>(), std::move(self));
    }

    static sp<VarType> absolute(sp<VarType> self) {
        return sp<VariableOP1<T>>::make(Operators::Abs<T>(), std::move(self));
    }

    static sp<VarType> normalize(sp<VarType> self) {
        return sp<VariableOP1<T>>::make(Operators::Normalize<T>(), std::move(self));
    }

    static sp<VarType> integral(sp<VarType> self, sp<Numeric> t) {
        sp<Numeric> duration = t ? std::move(t) : Ark::instance().appClock()->duration();
        return sp<Integral<T>>::make(std::move(self), std::move(duration));
    }

    static sp<VarType> integralS2(sp<VarType> self, const T& s0, const Optional<T>& s1, sp<Numeric> t) {
        sp<Numeric> duration = t ? std::move(t) : Ark::instance().appClock()->duration();
        return sp<IntegralS2<T>>::make(std::move(self), std::move(duration), s0, s1 ? s1.value() : s0);
    }

    static sp<VarType> integralWithResistance(sp<VarType> self, const T& v0, sp<Numeric> cd, sp<Numeric> t) {
        return sp<IntegralWithResistance<T>>::make(v0, std::move(self), cd, t ? std::move(t) : Ark::instance().appClock()->duration());
    }

    static sp<Numeric> distanceTo(sp<VarType> self, sp<VarType> other) {
        return Math::distance(std::move(self), std::move(other));
    }

    static void set(const sp<VariableWrapper<T>>& self, const T& val) {
        self->set(val);
    }

    static void set(const sp<VariableWrapper<T>>& self, sp<VarType> val) {
        self->set(std::move(val));
    }

    static void set(const sp<VarType>& self, const T& val) {
        ensureImpl(self)->set(val);
    }

    static sp<Numeric> x(const sp<VarType>& self) {
        const sp<IMPL> impl = self.template as<IMPL>();
        return impl ? static_cast<sp<Numeric>>(impl->x()) : sp<Numeric>::make<VariableOP1<float, T>>(Operators::Subscript<T, float>(0), self);
    }

    static void setX(const sp<VarType>& self, float x) {
        ensureImpl(self)->x()->set(x);
    }

    static void setX(const sp<VarType>& self, sp<Numeric> x) {
        ensureImpl(self)->x()->set(std::move(x));
    }

    static sp<Numeric> y(const sp<VarType>& self) {
        const sp<IMPL> impl = self.template as<IMPL>();
        return impl ? static_cast<sp<Numeric>>(impl->y()) : sp<Numeric>::make<VariableOP1<float, T>>(Operators::Subscript<T, float>(1), self);
    }

    static void setY(const sp<VarType>& self, float y) {
        ensureImpl(self)->y()->set(y);
    }

    static void setY(const sp<VarType>& self, sp<Numeric> y) {
        ensureImpl(self)->y()->set(std::move(y));
    }

    static sp<Numeric> z(const sp<VarType>& self) {
        if constexpr(DIMENSION > 2) {
            const sp<IMPL> impl = toImpl(self);
            return impl ? static_cast<sp<Numeric>>(impl->z()) : sp<Numeric>::make<VariableOP1<float, T>>(Operators::Subscript<T, float>(2), self);
        }
        else
            FATAL("You shouldn't be here");
        return nullptr;
    }

    template<typename U> static void setZ(const sp<VarType>& self, U z) {
        if constexpr(DIMENSION > 2)
            ensureImpl(self)->z()->set(std::move(z));
        else
            FATAL("You shouldn't be here");
    }

    static auto subscript(const sp<VarType>& self, size_t idx) -> remove_cvref_t<decltype(std::declval<T>()[0])> {
        CHECK(idx < DIMENSION, "Index(%uz) out of bounds", idx);
        return self->val()[idx];
    }

    static V2 xy(const sp<VarType>& self) {
        const T val = self->val();
        return V2(val[0], val[1]);
    }

    static V2 yx(const sp<VarType>& self) {
        const T val = self->val();
        return V2(val[1], val[0]);
    }

    static V3 xyz(const sp<VarType>& self) {
        const T val = self->val();
        CHECK(2 < DIMENSION, "Index z(3) out of bounds");
        return V3(val[0], val[1], val[2]);
    }

    [[deprecated]]
    static void fix(const sp<VarType>& self) {
        sp<VariableWrapper<T>> wrapper = self.template as<VariableWrapper<T>>();
        if(wrapper) {
            wrapper->fix();
            return;
        }
        sp<Vec2Impl> impl = self.template as<Vec2Impl>();
        CHECK(impl, "Object is not an instance of neither VariableWrapper<T> or Vec%dImpl", DIMENSION);
        ensureImpl(self)->fix();
    }

    static sp<VarType> freeze(const sp<VarType>& self) {
        return sp<typename VarType::Const>::make(self->val());
    }

    static sp<VarType> wrap(sp<VarType> self) {
        return sp<VariableWrapper<T>>::make(std::move(self));
    }

    static sp<VarType> synchronize(sp<VarType> self, sp<Boolean> disposed) {
        return Ark::instance().applicationContext()->synchronize(std::move(self), std::move(disposed));
    }

    static sp<VarType> modFloor(sp<VarType> self, sp<Numeric> mod) {
        return sp<VariableOP2<sp<VarType>, sp<VarType>, Operators::ModFloor<T>>>::make(std::move(self), sp<IMPL>::make(std::move(mod)));
    }

    static sp<VarType> modFloor(sp<VarType> self, sp<VarType> mod) {
        return sp<VariableOP2<sp<VarType>, sp<VarType>, Operators::ModFloor<T>>>::make(std::move(self), std::move(mod));
    }

    static sp<VarType> modCeil(sp<VarType> self, sp<Numeric> mod) {
        return sp<VariableOP2<sp<VarType>, sp<VarType>, Operators::ModCeil<T>>>::make(std::move(self), sp<IMPL>::make(std::move(mod)));
    }

    static sp<VarType> modCeil(sp<VarType> self, sp<VarType> mod) {
        return sp<VariableOP2<sp<VarType>, sp<VarType>, Operators::ModCeil<T>>>::make(self, std::move(mod));
    }

    static sp<VarType> ifElse(sp<VarType> self, sp<Boolean> condition, sp<VarType> otherwise) {
        return sp<VariableTernary<T>>::make(std::move(condition), std::move(self), std::move(otherwise));
    }

    static sp<VarType> wrapped(const sp<VarType>& self) {
        sp<VariableWrapper<T>> wrapper = self.template as<VariableWrapper<T>>();
        WARN(wrapper, "Non-Vec%dWrapper instance has no delegate attribute. This should be an error unless you're inspecting it.", DIMENSION);
        return wrapper ? wrapper->wrapped() : nullptr;
    }

    static sp<Size> toSize(const sp<VarType>& self) {
        return sp<Size>::make(x(self), y(self), DIMENSION > 2 ? z(self) : sp<Numeric>::make<Numeric::Const>(0.0f));
    }

    static sp<VarType> lerp(sp<VarType> self, sp<VarType> b, sp<Numeric> t) {
        if(!t)
            t = Ark::instance().appClock()->duration();
        return sp<Interpolate<T, float>>::make(std::move(self), std::move(b), std::move(t));
    }

    static sp<VarType> sod(sp<VarType> self, float k, float z, float r, sp<Numeric> t) {
        if(!t)
            t = Ark::instance().appClock()->duration();
        return sp<SecondOrderDynamics<T>>::make(std::move(self), std::move(t), k, z, r);
    }

protected:
    static sp<IMPL> toImpl(const sp<VarType>& self) {
        return self.template as<IMPL>();
    }

    static sp<IMPL> ensureImpl(const sp<VarType>& self) {
        sp<IMPL> impl = self.template as<IMPL>();
        CHECK(impl, "This Vec%d object is not a Vec%dImpl instance", DIMENSION, DIMENSION);
        return impl;
    }
};

}

#endif