#include "graphics/components/rotation.h"

#include <glm/gtx/quaternion.hpp>

#include "core/base/bean_factory.h"
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
        update(Timestamp::now());
    }

    bool update(const uint64_t timestamp) override
    {
        if(_quaternion->update(timestamp))
        {
            const V4 quaternion = _quaternion->val();
            _matrix = {glm::toMat4(glm::quat(quaternion.w(), quaternion.x(), quaternion.y(), quaternion.z()))};
            return true;
        }
        return false;
    }

    M4 val() override
    {
        return _matrix;
    }

private:
    sp<Vec4> _quaternion;
    M4 _matrix;
};

class Vec3MulitplyPosition final : public Vec3 {
public:
    Vec3MulitplyPosition(sp<Vec4> quaternion, sp<Vec3> position)
        : _quaternion(std::move(quaternion)), _position(std::move(position)) {
        update(Timestamp::now());
    }

    bool update(const uint64_t timestamp) override
    {
        if(UpdatableUtil::update(timestamp, _quaternion, _position))
        {
            const V4 quat = _quaternion->val();
            const V3 dir = _position->val();
            const glm::vec3 v = glm::rotate(glm::quat(quat.w(), quat.x(), quat.y(), quat.z()), glm::vec3(dir.x(), dir.y(), dir.z()));
            _value = V3(v.x, v.y, v.z);
            return true;
        }
        return false;
    }

    V3 val() override
    {
        return _value;
    }

private:
    sp<Vec4> _quaternion;
    sp<Vec3> _position;
    V3 _value;
};

class Vec4MulitplyPosition final : public Vec4 {
public:
    Vec4MulitplyPosition(sp<Vec4> quaternion, sp<Vec4> position)
        : _quaternion(std::move(quaternion)), _position(std::move(position)) {
        update(Timestamp::now());
    }

    bool update(const uint64_t timestamp) override
    {
        if(UpdatableUtil::update(timestamp, _quaternion, _position))
        {
            const V4 quat = _quaternion->val();
            const V4 dir = _position->val();
            const glm::vec4 v = glm::rotate(glm::quat(quat.w(), quat.x(), quat.y(), quat.z()), glm::vec4(dir.x(), dir.y(), dir.z(), dir.w()));
            _value = V4(v.x, v.y, v.z, v.w);
            return true;
        }
        return false;
    }

    V4 val() override
    {
        return _value;
    }

private:
    sp<Vec4> _quaternion;
    sp<Vec4> _position;
    V4 _value;
};

class Vec4MulitplyQuaternion final : public Vec4 {
public:
    Vec4MulitplyQuaternion(sp<Vec4> quaternion1, sp<Vec4> quaternion2)
        : _quaternion1(std::move(quaternion1)), _quaternion2(std::move(quaternion2)) {
        update(Timestamp::now());
    }

    bool update(const uint64_t timestamp) override
    {
        if(UpdatableUtil::update(timestamp, _quaternion1, _quaternion2))
        {
            const V4 quat1 = _quaternion1->val();
            const V4 quat2 = _quaternion2->val();
            const glm::quat v = glm::quat(quat1.w(), quat1.x(), quat1.y(), quat1.z()) * glm::quat(quat2.w(), quat2.x(), quat2.y(), quat2.z());
            _value = V4(v.x, v.y, v.z, v.w);
            return true;
        }
        return false;
    }

    V4 val() override
    {
        return _value;
    }

private:
    sp<Vec4> _quaternion1;
    sp<Vec4> _quaternion2;
    V4 _value;
};

class VectorReorientQuaternion final : public Vec4 {
public:
    VectorReorientQuaternion(sp<Vec3> u, sp<Vec3> v)
        : _u(std::move(u)), _v(std::move(v))
    {
        update(Timestamp::now());
    }

    V4 val() override
    {
        return _value;
    }

    bool update(const uint64_t timestamp) override
    {
        if(UpdatableUtil::update(timestamp, _u, _v))
        {
            const V3 u = _u->val();
            const V3 v = _v->val();
            const glm::quat q(glm::vec3(u.x(), u.y(), u.z()), glm::vec3(v.x(), v.y(), v.z()));
            _value = V4(q.x, q.y, q.z, q.w);
            return true;
        }
        return false;
    }

private:
    void doUpdate();

private:
    sp<Vec3> _u;
    sp<Vec3> _v;
    V4 _value;
};

}

Rotation::Rotation(const V4 quaternion)
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

bool Rotation::update(const uint64_t timestamp)
{
    return _delegate->update(timestamp);
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
    return sp<Vec3>::make<Vec3MulitplyPosition>(_delegate, std::move(v));
}

sp<Vec4> Rotation::applyTo(sp<Vec4> v) const
{
    return sp<Vec4>::make<Vec4MulitplyPosition>(_delegate, std::move(v));
}

sp<Mat4> Rotation::toMatrix() const
{
    return sp<Mat4>::make<Mat4Quaternion>(_delegate);
}

sp<Rotation> Rotation::freeze() const
{
    return sp<Rotation>::make(_delegate->val());
}

sp<Rotation> Rotation::mul(sp<Rotation> lhs, sp<Rotation> rhs)
{
    return sp<Rotation>::make(sp<Vec4>::make<Vec4MulitplyQuaternion>(lhs, rhs));
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
    return sp<Rotation>::make(sp<Vec4>::make<VectorReorientQuaternion>(std::move(u), std::move(v)));
}

}
