#include "graphics/components/rotation.h"

#include <glm/gtx/quaternion.hpp>

#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_cached.h"
#include "core/impl/variable/variable_dirty_mark.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/util/updatable_util.h"

#include "graphics/base/rotation_axis_theta.h"
#include "graphics/base/rotation_euler.h"
#include "graphics/base/mat.h"

namespace ark {

namespace {

class Mat4Quaternion final : public Mat4 {
public:
    Mat4Quaternion(sp<Vec4> quaternion)
        : _quaternion(std::move(quaternion)) {
    }

    bool update(const uint32_t tick) override
    {
        return _quaternion->update(tick);
    }

    M4 val() override
    {
        const V4 quaternion = _quaternion->val();
        return {glm::toMat4(glm::quat(quaternion.w(), quaternion.x(), quaternion.y(), quaternion.z()))};
    }

private:
    sp<Vec4> _quaternion;
};

class Vec3MulitplyPosition final : public Vec3 {
public:
    Vec3MulitplyPosition(sp<Vec4> quaternion, sp<Vec3> position)
        : _quaternion(std::move(quaternion)), _position(std::move(position)) {
    }

    bool update(const uint32_t tick) override
    {
        return UpdatableUtil::update(tick, _quaternion, _position);
    }

    V3 val() override
    {
        const V4 quat = _quaternion->val();
        const V3 dir = _position->val();
        const glm::vec3 v = glm::rotate(glm::quat(quat.w(), quat.x(), quat.y(), quat.z()), glm::vec3(dir.x(), dir.y(), dir.z()));
        return {v.x, v.y, v.z};
    }

private:
    sp<Vec4> _quaternion;
    sp<Vec3> _position;
};

class Vec4MulitplyPosition final : public Vec4 {
public:
    Vec4MulitplyPosition(sp<Vec4> quaternion, sp<Vec4> position)
        : _quaternion(std::move(quaternion)), _position(std::move(position)) {
    }

    bool update(const uint32_t tick) override
    {
        return UpdatableUtil::update(tick, _quaternion, _position);
    }

    V4 val() override
    {
        const V4 quat = _quaternion->val();
        const V4 dir = _position->val();
        const glm::vec4 v = glm::rotate(glm::quat(quat.w(), quat.x(), quat.y(), quat.z()), glm::vec4(dir.x(), dir.y(), dir.z(), dir.w()));
        return {v.x, v.y, v.z, v.w};
    }

private:
    sp<Vec4> _quaternion;
    sp<Vec4> _position;
};

class Vec4MulitplyQuaternion final : public Vec4 {
public:
    Vec4MulitplyQuaternion(sp<Vec4> quaternion1, sp<Vec4> quaternion2)
        : _quaternion1(std::move(quaternion1)), _quaternion2(std::move(quaternion2)) {
    }

    bool update(const uint32_t tick) override
    {
        return UpdatableUtil::update(tick, _quaternion1, _quaternion2);
    }

    V4 val() override
    {
        const V4 quat1 = _quaternion1->val();
        const V4 quat2 = _quaternion2->val();
        const glm::quat v = glm::quat(quat1.w(), quat1.x(), quat1.y(), quat1.z()) * glm::quat(quat2.w(), quat2.x(), quat2.y(), quat2.z());
        return {v.x, v.y, v.z, v.w};
    }

private:
    sp<Vec4> _quaternion1;
    sp<Vec4> _quaternion2;
};

class Vec4ReorientQuaternion final : public Vec4 {
public:
    Vec4ReorientQuaternion(sp<Vec3> u, sp<Vec3> v)
        : _u(std::move(u)), _v(std::move(v)) {
    }

    bool update(const uint32_t tick) override
    {
        return UpdatableUtil::update(tick, _u, _v);
    }

    V4 val() override
    {
        const V3 u = _u->val();
        const V3 v = _v->val();
        const glm::quat q(glm::vec3(u.x(), u.y(), u.z()), glm::vec3(v.x(), v.y(), v.z()));
        return {q.x, q.y, q.z, q.w};
    }

private:
    sp<Vec3> _u;
    sp<Vec3> _v;
};

}

Rotation::Rotation(const V4& quaternion)
    : Rotation(sp<Vec4>::make<Vec4::Const>(quaternion))
{
}

Rotation::Rotation(sp<Vec4> quaternion)
    : Rotation(sp<Vec4Wrapper>::make(std::move(quaternion)))
{
}

Rotation::Rotation(const sp<Vec4Wrapper>& vec4Wrapper)
    : _delegate(vec4Wrapper), _wrapper(vec4Wrapper)
{
}

Rotation::Rotation(sp<Vec4> delegate, sp<Wrapper<Vec4>> wrapper)
    : _delegate(std::move(delegate)), _wrapper(std::move(wrapper))
{
}

V4 Rotation::val()
{
    return _delegate->val();
}

bool Rotation::update(uint32_t tick)
{
    return _delegate->update(tick);
}

void Rotation::reset(sp<Vec4> quaternion)
{
    VariableDirtyMark<V4>::markDirty(_wrapper, std::move(quaternion));
}

void Rotation::setAxisTheta(const V3 axis, const float theta)
{
    setAxisTheta(sp<Vec3>::make<Vec3::Const>(axis), sp<Numeric>::make<Numeric::Const>(theta));
}

void Rotation::setAxisTheta(sp<Vec3> axis, sp<Numeric> theta)
{
    reset(sp<Vec4>::make<RotationAxisTheta>(std::move(axis), std::move(theta)));
}

void Rotation::setEuler(const float pitch, const float yaw, const float roll)
{
    setEuler(sp<Numeric::Const>::make(pitch), sp<Numeric::Const>::make(yaw), sp<Numeric::Const>::make(roll));
}

void Rotation::setEuler(sp<Numeric> pitch, sp<Numeric> yaw, sp<Numeric> roll)
{
    reset(sp<Vec4>::make<RotationEuler>(std::move(pitch), std::move(yaw), std::move(roll)));
}

sp<Vec3> Rotation::applyTo(sp<Vec3> v) const
{
    return VariableCached<V3>::create<Vec3MulitplyPosition>(_delegate, std::move(v));
}

sp<Vec4> Rotation::applyTo(sp<Vec4> v) const
{
    return VariableCached<V4>::create<Vec4MulitplyPosition>(_delegate, std::move(v));
}

sp<Mat4> Rotation::toMatrix() const
{
    return VariableCached<M4>::create<Mat4Quaternion>(_delegate);
}

sp<Rotation> Rotation::freeze() const
{
    return sp<Rotation>::make(_delegate->val());
}

sp<Rotation> Rotation::mul(sp<Rotation> lhs, sp<Rotation> rhs)
{
    return sp<Rotation>::make(VariableCached<V4>::create<Vec4MulitplyQuaternion>(lhs, rhs));
}

sp<Rotation> Rotation::axisTheta(sp<Vec3> axis, sp<Numeric> theta)
{
    return sp<Rotation>::make(sp<Vec4>::make<RotationAxisTheta>(std::move(axis), std::move(theta)));
}

sp<Rotation> Rotation::eulerAngle(sp<Numeric> pitch, sp<Numeric> yaw, sp<Numeric> roll)
{
    return sp<Rotation>::make(sp<Vec4>::make<RotationEuler>(std::move(pitch), std::move(yaw), std::move(roll)));
}

sp<Rotation> Rotation::vectorReorientation(sp<Vec3> u, sp<Vec3> v)
{
    return sp<Rotation>::make(VariableCached<V4>::create<Vec4ReorientQuaternion>(std::move(u), std::move(v)));
}

}
