#include "graphics/util/vec4_type.h"

#include "core/impl/variable/variable_op1.h"
#include "core/util/operators.h"

#include "graphics/impl/vec/vec4_impl.h"

namespace ark {

sp<Vec4> Vec4Type::create(float x, float y, float z, float w)
{
    return sp<Vec4Impl>::make(x, y, z, w);
}

sp<Vec4> Vec4Type::create(sp<Numeric> x, sp<Numeric> y, sp<Numeric> z, sp<Numeric> w)
{
    return sp<Vec4Impl>::make(std::move(x), std::move(y), std::move(z), std::move(w));
}

sp<Numeric> Vec4Type::w(const sp<Vec4>& self)
{
    const sp<Vec4Impl> impl = self.tryCast<Vec4Impl>();
    return impl ? static_cast<sp<Numeric>>(impl->w()) : sp<Numeric>::make<VariableOP1<float, V4>>(Operators::Subscript<V4, float>(3), self);
}

void Vec4Type::setW(const sp<Vec4>& self, float w)
{
    ensureImpl(self)->w()->set(w);
}

void Vec4Type::setW(const sp<Vec4>& self, sp<Numeric> w)
{
    ensureImpl(self)->w()->set(std::move(w));
}

sp<Vec4> Vec4Type::xyzw(sp<Vec4> self)
{
    return self;
}

template<> ARK_API sp<Vec4> Null::safePtr()
{
    return sp<Vec4Impl>::make();
}

}
