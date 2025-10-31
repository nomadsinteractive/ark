#include "graphics/util/vec3_type.h"

#include <glm/glm.hpp>

#include "core/util/operators.h"
#include "core/util/updatable_util.h"

namespace ark {

namespace {

class Vec3Cross final : public Vec3 {
public:
    Vec3Cross(sp<Vec3> a, sp<Vec3> b)
        : _a(std::move(a)), _b(std::move(b)) {
    }

    V3 val() override {
        return _a->val().cross(_b->val());
    }

    bool update(uint32_t tick) override {
        return UpdatableUtil::update(tick, _a, _b);
    }

private:
    sp<Vec3> _a;
    sp<Vec3> _b;
};

class Vec3Normalize final : public Wrapper<Vec3>, public Vec3, Implements<Vec3Normalize, Vec3, Wrapper<Vec3>> {
public:
    Vec3Normalize(sp<Vec3> delegate)
        : Wrapper(std::move(delegate)) {
    }

    V3 val() override {
        const V3 v = _wrapped->val();
        const glm::vec3 normalized = glm::normalize(glm::vec3(v.x(), v.y(), v.z()));
        return {normalized.x, normalized.y, normalized.z};
    }

    bool update(uint32_t tick) override {
        return _wrapped->update(tick);
    }
};

}

sp<Vec3> Vec3Type::create(sp<Numeric> x, sp<Numeric> y, sp<Numeric> z)
{
    ASSERT((x && y && z) || (x && !y && !z));
    if(!y)
        return wrap(sp<Vec3>::make<Vec3Impl>(std::move(x)));
    return wrap(sp<Vec3>::make<Vec3Impl>(std::move(x), std::move(y), std::move(z)));
}

sp<Vec3> Vec3Type::create(float x, float y, float z)
{
    return wrap(sp<Vec3>::make<Vec3Impl>(x, y, z));
}

sp<Vec4> Vec3Type::extend(sp<Vec3> self, sp<Numeric> w)
{
    return sp<Vec4>::make<VariableOP2<sp<Vec3>, sp<Numeric>, Operators::Extend<V3, float>>>(std::move(self), std::move(w));
}

sp<Vec3> Vec3Type::cross(sp<Vec3> self, sp<Vec3> other)
{
    return sp<Vec3>::make<Vec3Cross>(std::move(self), std::move(other));
}

sp<Vec3> Vec3Type::cross(sp<Vec3> self, const V3& other)
{
    return sp<Vec3>::make<Vec3Cross>(std::move(self), sp<Vec3::Const>::make(other));
}

}
