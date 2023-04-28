#include "graphics/base/camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "core/ark.h"

#include "core/impl/variable/variable_wrapper.h"
#include "core/impl/writable/writable_memory.h"
#include "core/inf/writable.h"
#include "core/types/global.h"
#include "core/util/updatable_util.h"

#include "graphics/base/mat.h"
#include "graphics/base/viewport.h"
#include "graphics/util/matrix_util.h"
#include "graphics/util/mat4_type.h"
#include "graphics/util/vec3_type.h"

#include "renderer/base/render_engine.h"

#include "app/base/application_context.h"
#include "app/base/application_manifest.h"

namespace ark {

namespace {

class OrthoMatrixVariable : public Mat4 {
public:
    OrthoMatrixVariable(sp<Camera::Delegate> delegate, sp<Vec2> leftTop, sp<Vec2> rightBottom, sp<Vec2> clip)
        : _delegate(std::move(delegate)), _left_top(std::move(leftTop)), _right_bottom(std::move(rightBottom)), _clip(std::move(clip)),
          _matrix(calcMatrix()) {
    }

    virtual M4 val() override {
        return _matrix;
    }

    virtual bool update(uint64_t timestamp) override {
        if(UpdatableUtil::update(timestamp, _left_top, _right_bottom, _clip)) {
            _matrix = calcMatrix();
            return true;
        }
        return false;
    }

private:
    M4 calcMatrix() const {
        const V2 leftTop = _left_top->val();
        const V2 rightBottom = _right_bottom->val();
        const V2 clip = _clip->val();
        return _delegate->ortho(leftTop.x(), rightBottom.x(), rightBottom.y(), leftTop.y(), clip.x(), clip.y());
    }

private:
    sp<Camera::Delegate> _delegate;

    sp<Vec2> _left_top;
    sp<Vec2> _right_bottom;
    sp<Vec2> _clip;

    M4 _matrix;
};

class FrustumMatrixVariable : public Mat4 {
public:
    FrustumMatrixVariable(sp<Camera::Delegate> delegate, sp<Vec3> position, sp<Vec3> target, sp<Vec3> up)
        : _delegate(std::move(delegate)), _position(std::move(position)), _target(std::move(target)), _up(std::move(up)),
          _matrix(_delegate->lookAt(_position->val(), _target->val(), _up->val())) {
    }

    virtual M4 val() override {
        return _matrix;
    }

    virtual bool update(uint64_t timestamp) override {
        if(UpdatableUtil::update(timestamp, _position, _target, _up)) {
            _matrix = _delegate->lookAt(_position->val(), _target->val(), _up->val());
            return true;
        }
        return false;
    }

private:
    sp<Camera::Delegate> _delegate;

    sp<Vec3> _position;
    sp<Vec3> _target;
    sp<Vec3> _up;

    M4 _matrix;
};

class MulMatrixVariable : public Mat4 {
public:
    MulMatrixVariable(sp<Mat4> lvalue, sp<Mat4> rvalue)
        : _lvalue(std::move(lvalue)), _rvalue(std::move(rvalue)), _matrix(MatrixUtil::mul(_lvalue->val(), _rvalue->val())) {
    }

    virtual M4 val() override {
        return _matrix;
    }

    virtual bool update(uint64_t timestamp) override {
        if(UpdatableUtil::update(timestamp, _lvalue, _rvalue)) {
            _matrix = MatrixUtil::mul(_lvalue->val(), _rvalue->val());
            return true;
        }
        return false;
    }

private:
    sp<Mat4> _lvalue;
    sp<Mat4> _rvalue;

    M4 _matrix;
};

}

static const char sclipNearclipFarPlaneWarning[] = "ClipNear: %.2f, ClipFar: %.2f. Far plane should be further than near plane, and distance to the near plane should be greater than zero.";

Camera::Camera()
    : Camera(Ark::instance().applicationContext()->renderController()->createCamera())
{
}

Camera::Camera(Ark::RendererCoordinateSystem cs, sp<Delegate> delegate)
    : _coordinate_system(cs), _delegate(std::move(delegate)), _view(sp<Holder>::make(sp<Mat4::Const>::make(M4()))), _projection(sp<Holder>::make(sp<Mat4::Const>::make(M4()))),
      _vp(sp<Holder>::make(sp<Mat4::Const>::make(M4()))), _position(sp<VariableWrapper<V3>>::make(V3(0))), _target(sp<VariableWrapper<V3>>::make(V3(0)))
{
}

void Camera::ortho(const V2& leftTop, const V2& rightBottom, const V2& clip)
{
    ortho(leftTop.x(), rightBottom.x(), rightBottom.y(), leftTop.y(), clip.x(), clip.y());
}

void Camera::ortho(sp<Vec2> leftTop, sp<Vec2> rightBottom, sp<Vec2> clip)
{
    _projection->setMatrix(sp<OrthoMatrixVariable>::make(_delegate, std::move(leftTop), std::move(rightBottom), std::move(clip)));
    updateViewProjection();
}

void Camera::ortho(float left, float right, float bottom, float top, float clipNear, float clipFar)
{
    _projection->setMatrix(sp<Mat4::Const>::make(_delegate->ortho(left, right, bottom, top, clipNear, clipFar)));
    updateViewProjection();
}

void Camera::ortho(float left, float right, float bottom, float top, float clipNear, float clipFar, Ark::RendererCoordinateSystem coordinateSystem)
{
    if(coordinateSystem == Ark::COORDINATE_SYSTEM_LHS)
    {
        std::swap(top, bottom);
        std::swap(clipNear, clipFar);
    }
    ortho(left, right, bottom, top, clipNear, clipFar);
}

void Camera::frustum(float left, float right, float bottom, float top, float clipNear, float clipFar)
{
    CHECK_WARN(clipFar > clipNear && clipNear > 0, sclipNearclipFarPlaneWarning, clipNear, clipFar);
    _projection->setMatrix(sp<Mat4::Const>::make(_delegate->frustum(left, right, bottom, top, clipNear, clipFar)));
    updateViewProjection();
}

void Camera::perspective(float fov, float aspect, float clipNear, float clipFar)
{
    CHECK_WARN(clipFar > clipNear && clipNear > 0, sclipNearclipFarPlaneWarning, clipNear, clipFar);
    _projection->setMatrix(sp<Mat4::Const>::make(_delegate->perspective(fov, aspect, clipNear, clipFar)));
    updateViewProjection();
}

void Camera::lookAt(const V3& position, const V3& target, const V3& up)
{
    _position->set(position);
    _target->set(target);
    _view->setMatrix(sp<Mat4::Const>::make(_delegate->lookAt(position, target, up)));
    updateViewProjection();
}

void Camera::lookAt(const sp<Vec3>& position, const sp<Vec3>& target, const sp<Vec3>& up)
{
    _position->set(position);
    _target->set(target);
    _view->setMatrix(sp<FrustumMatrixVariable>::make(_delegate, position, target, up));
    updateViewProjection();
}

V3 Camera::toWorldPosition(float screenX, float screenY, float z) const
{
    M4 vp;
    WritableMemory wm(&vp);
    _vp->upload(wm);
    return Ark::instance().applicationContext()->renderEngine()->toWorldPosition(vp, screenX, screenY, z);
}

sp<Vec3> Camera::toViewportPosition(const sp<Vec3>& position) const
{
    const Viewport& viewport = Ark::instance().applicationContext()->renderEngine()->viewport();
    const V3 scale = Ark::instance().manifest()->renderer()._coordinate_system == _coordinate_system ? V3(0.5f, 0.5f, 0.5f) : V3(0.5f, -0.5f, 0.5f);
    return Vec3Type::mul(Vec3Type::add(Vec3Type::mul(Mat4Type::matmul(_vp, position), scale), V3(0.5f, 0.5f, 0.5f)), V3(viewport.width(), viewport.height(), 1.0f));
}

sp<Vec3> Camera::position() const
{
    return _position;
}

sp<Vec3> Camera::target() const
{
    return _target;
}

sp<Mat4> Camera::matrixView() const
{
    return _view;
}

sp<Mat4> Camera::matrixProjection() const
{
    return _projection;
}

sp<Mat4> Camera::matrixViewProjection() const
{
    return _vp;
}

void Camera::assign(const Camera& other)
{
    _view->setMatrix(other._view);
    _projection->setMatrix(other._projection);
    _vp->setMatrix(other._vp);
}

const sp<Camera::Holder>& Camera::view() const
{
    return _view;
}

const sp<Camera::Holder>& Camera::projection() const
{
    return _projection;
}

const sp<Camera::Holder>& Camera::vp() const
{
    return _vp;
}

const sp<Camera>& Camera::getDefaultCamera()
{
    const Global<Camera> camera;
    DCHECK(camera->vp(), "Default camera has not been uninitialized");
    return camera;
}

void Camera::updateViewProjection()
{
    _vp->setMatrix(sp<MulMatrixVariable>::make(_projection->matrix(), _view->matrix()));
}

Camera::Holder::Holder(sp<Mat4> value)
    : Uploader(sizeof(M4)), _matrix(std::move(value)) {
}

void Camera::Holder::upload(Writable& buf)
{
    buf.writeObject(_matrix->val());
}

bool Camera::Holder::update(uint64_t timestamp)
{
    return _matrix->update(timestamp) || _timestamp.update(timestamp);
}

M4 Camera::Holder::val()
{
    return _matrix->val();
}

const sp<Mat4>& Camera::Holder::matrix() const
{
    return _matrix;
}

void Camera::Holder::setMatrix(sp<Mat4> matrix)
{
    _matrix = std::move(matrix);
    _timestamp.markDirty();
}

M4 Camera::DelegateLH_ZO::frustum(float left, float right, float bottom, float top, float clipNear, float clipFar)
{
    return M4(glm::frustumLH_ZO(left, right, bottom, top, clipNear, clipFar));
}

M4 Camera::DelegateLH_ZO::lookAt(const V3& position, const V3& target, const V3& up)
{
    return M4(glm::lookAtLH(glm::vec3(position.x(), position.y(), position.z()), glm::vec3(target.x(), target.y(), target.z()), glm::vec3(-up.x(), -up.y(), -up.z())));
}

M4 Camera::DelegateLH_ZO::ortho(float left, float right, float bottom, float top, float clipNear, float clipFar)
{
    return M4(glm::orthoLH_ZO(left, right, bottom, top, clipNear, clipFar));
}

M4 Camera::DelegateLH_ZO::perspective(float fov, float aspect, float clipNear, float clipFar)
{
    return M4(glm::perspectiveLH_ZO(fov, aspect, clipNear, clipFar));
}

M4 Camera::DelegateRH_ZO::frustum(float left, float right, float bottom, float top, float clipNear, float clipFar)
{
    return M4(glm::frustumRH_ZO(left, right, bottom, top, clipNear, clipFar));
}

M4 Camera::DelegateRH_ZO::lookAt(const V3& position, const V3& target, const V3& up)
{
    return M4(glm::lookAtRH(glm::vec3(position.x(), position.y(), position.z()), glm::vec3(target.x(), target.y(), target.z()), glm::vec3(-up.x(), -up.y(), -up.z())));
}

M4 Camera::DelegateRH_ZO::ortho(float left, float right, float bottom, float top, float clipNear, float clipFar)
{
    return M4(glm::orthoRH_ZO(left, right, bottom, top, clipNear, clipFar));
}

M4 Camera::DelegateRH_ZO::perspective(float fov, float aspect, float clipNear, float clipFar)
{
    return M4(glm::perspectiveRH_ZO(fov, aspect, clipNear, clipFar));
}

M4 Camera::DelegateLH_NO::frustum(float left, float right, float bottom, float top, float clipNear, float clipFar)
{
    return M4(glm::frustumLH_NO(left, right, bottom, top, clipNear, clipFar));
}

M4 Camera::DelegateLH_NO::lookAt(const V3& position, const V3& target, const V3& up)
{
    return M4(glm::lookAtLH(glm::vec3(position.x(), position.y(), position.z()), glm::vec3(target.x(), target.y(), target.z()), glm::vec3(up.x(), up.y(), up.z())));
}

M4 Camera::DelegateLH_NO::ortho(float left, float right, float bottom, float top, float clipNear, float clipFar)
{
    return M4(glm::orthoLH_NO(left, right, bottom, top, clipNear, clipFar));
}

M4 Camera::DelegateLH_NO::perspective(float fov, float aspect, float clipNear, float clipFar)
{
    return M4(glm::perspectiveLH_NO(fov, aspect, clipNear, clipFar));
}

M4 Camera::DelegateRH_NO::frustum(float left, float right, float bottom, float top, float clipNear, float clipFar)
{
    return M4(glm::frustumRH_NO(left, right, bottom, top, clipNear, clipFar));
}

M4 Camera::DelegateRH_NO::lookAt(const V3& position, const V3& target, const V3& up)
{
    return M4(glm::lookAtRH(glm::vec3(position.x(), position.y(), position.z()), glm::vec3(target.x(), target.y(), target.z()), glm::vec3(up.x(), up.y(), up.z())));
}

M4 Camera::DelegateRH_NO::ortho(float left, float right, float bottom, float top, float clipNear, float clipFar)
{
    return M4(glm::orthoRH_NO(left, right, bottom, top, clipNear, clipFar));
}

M4 Camera::DelegateRH_NO::perspective(float fov, float aspect, float clipNear, float clipFar)
{
    return M4(glm::perspectiveRH_NO(fov, aspect, clipNear, clipFar));
}

}
