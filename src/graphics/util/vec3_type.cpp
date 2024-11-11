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

    bool update(uint64_t timestamp) override {
        return UpdatableUtil::update(timestamp, _a, _b);
    }

private:
    sp<Vec3> _a;
    sp<Vec3> _b;

};

class Vec3Normalize : public Wrapper<Vec3>, public Vec3, Implements<Vec3Normalize, Vec3, Wrapper<Vec3>> {
public:
    Vec3Normalize(sp<Vec3> delegate)
        : Wrapper(std::move(delegate)) {
    }

    virtual V3 val() override {
        const V3 v = _wrapped->val();
        const glm::vec3 normalized = glm::normalize(glm::vec3(v.x(), v.y(), v.z()));
        return V3(normalized.x, normalized.y, normalized.z);
    }

    virtual bool update(uint64_t timestamp) override {
        return _wrapped->update(timestamp);
    }

};

}

sp<Vec3> Vec3Type::create(sp<Numeric> x, sp<Numeric> y, sp<Numeric> z)
{
    return sp<Vec3Impl>::make(std::move(x), std::move(y), std::move(z));
}

sp<Vec3> Vec3Type::create(float x, float y, float z)
{
    return sp<Vec3Impl>::make(x, y, z);
}

sp<Vec4> Vec3Type::extend(sp<Vec3> self, sp<Numeric> w)
{
    return sp<VariableOP2<sp<Vec3>, sp<Numeric>, Operators::Extend<V3, float>>>::make(std::move(self), std::move(w));
}

sp<Vec3> Vec3Type::cross(sp<Vec3> self, sp<Vec3> other)
{
    return sp<Vec3Cross>::make(std::move(self), std::move(other));
}

sp<Vec3> Vec3Type::cross(sp<Vec3> self, const V3& other)
{
    return sp<Vec3Cross>::make(std::move(self), sp<Vec3::Const>::make(other));
}

}
