#include "graphics/base/camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "core/ark.h"

#include "core/base/observer.h"
#include "core/epi/notifier.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/impl/boolean/boolean_by_weak_ref.h"
#include "core/impl/variable/variable_op2.h"
#include "core/types/global.h"
#include "core/util/operators.h"
#include "core/util/variable_util.h"

#include "graphics/base/mat.h"
#include "graphics/base/viewport.h"
#include "graphics/util/matrix_util.h"

#include "renderer/base/render_engine.h"

#include "app/base/application_context.h"

namespace ark {

namespace {

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
        if(VariableUtil::update(timestamp, _position, _target, _up)) {
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
        if(VariableUtil::update(timestamp, _lvalue, _rvalue)) {
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

static const char sNearFarPlaneWarning[] = "Near: %.2f, Far: %.2f. Far plane should be further than near plane, and distance to the near plane should be greater than zero.";

Camera::Camera()
    : Camera(Ark::instance().applicationContext()->renderController()->createCamera())
{
}

Camera::Camera(Ark::RendererCoordinateSystem cs, sp<Delegate> delegate)
    : _coordinate_system(cs), _delegate(std::move(delegate)), _view(sp<Holder>::make(sp<Mat4::Const>::make(M4()))), _projection(sp<Holder>::make(sp<Mat4::Const>::make(M4()))),
      _vp(sp<Holder>::make(sp<Mat4::Const>::make(M4()))), _position(sp<VariableWrapper<V3>>::make(V3(0))), _target(sp<VariableWrapper<V3>>::make(V3(0)))
{
}

void Camera::ortho(float left, float right, float bottom, float top, float near, float far)
{
    ortho(left, right, bottom, top, near, far, _coordinate_system);
}

void Camera::ortho(float left, float right, float bottom, float top, float near, float far, Ark::RendererCoordinateSystem coordinateSystem)
{
    if(coordinateSystem  < 0)
        std::swap(top, bottom);

    _vp->setMatrix(sp<Mat4::Const>::make(_delegate->ortho(left, right, bottom, top, near * 2 - far, far)));
}

void Camera::frustum(float left, float right, float bottom, float top, float near, float far)
{
    DWARN(far > near && near > 0, sNearFarPlaneWarning, near, far);
    _projection->setMatrix(sp<Mat4::Const>::make(_delegate->frustum(left, right, bottom, top, near, far)));
    updateViewProjection();
}

void Camera::perspective(float fov, float aspect, float near, float far)
{
    DWARN(far > near && near > 0, sNearFarPlaneWarning, near, far);
    _projection->setMatrix(sp<Mat4::Const>::make(_delegate->perspective(fov, aspect, near, far)));
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

V3 Camera::getRayDirection(float screenX, float screenY) const
{
    M4 vp;
    _vp->flat(&vp);

    const Viewport& viewport = Ark::instance().applicationContext()->renderEngine()->viewport();

    float ndcx = (screenX * 2 - viewport.width()) / viewport.width();
    float ndcy = (screenY * 2 - viewport.height()) / viewport.height();
    const V4 pos = MatrixUtil::mul(MatrixUtil::inverse(vp), V4(ndcx, _coordinate_system == Ark::COORDINATE_SYSTEM_LHS ? -ndcy : ndcy, 1.0f, 1.0f));
    return V3(pos.x() / pos.w(), pos.y() / pos.w(), pos.z() / pos.w());
}

sp<Vec3> Camera::position() const
{
    return _position;
}

sp<Vec3> Camera::target() const
{
    return _target;
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
    : _matrix(std::move(value)) {
}

void Camera::Holder::flat(void* buf)
{
    *reinterpret_cast<M4*>(buf) = _matrix->val();
}

uint32_t Camera::Holder::size()
{
    return sizeof(M4);
}

bool Camera::Holder::update(uint64_t timestamp)
{
    return _matrix->update(timestamp) || _timestamp.update(timestamp);
}

const sp<Mat4>& Camera::Holder::matrix() const
{
    return _matrix;
}

void Camera::Holder::setMatrix(sp<Mat4> matrix)
{
    _matrix = std::move(matrix);
    _timestamp.setDirty();
}

M4 Camera::DelegateLH_ZO::frustum(float left, float right, float bottom, float top, float near, float far)
{
    return M4(glm::frustumLH_ZO(left, right, bottom, top, near, far));
}

M4 Camera::DelegateLH_ZO::lookAt(const V3& position, const V3& target, const V3& up)
{
    return M4(glm::lookAtLH(glm::vec3(position.x(), position.y(), position.z()), glm::vec3(target.x(), target.y(), target.z()), glm::vec3(-up.x(), -up.y(), -up.z())));
}

M4 Camera::DelegateLH_ZO::ortho(float left, float right, float bottom, float top, float near, float far)
{
    return M4(glm::orthoLH_ZO(left, right, bottom, top, near, far));
}

M4 Camera::DelegateLH_ZO::perspective(float fov, float aspect, float near, float far)
{
    return M4(glm::perspectiveLH_ZO(fov, aspect, near, far));
}

M4 Camera::DelegateRH_ZO::frustum(float left, float right, float bottom, float top, float near, float far)
{
    return M4(glm::frustumRH_ZO(left, right, bottom, top, near, far));
}

M4 Camera::DelegateRH_ZO::lookAt(const V3& position, const V3& target, const V3& up)
{
    return M4(glm::lookAtRH(glm::vec3(position.x(), position.y(), position.z()), glm::vec3(target.x(), target.y(), target.z()), glm::vec3(-up.x(), -up.y(), -up.z())));
}

M4 Camera::DelegateRH_ZO::ortho(float left, float right, float bottom, float top, float near, float far)
{
    return M4(glm::orthoRH_ZO(left, right, bottom, top, near, far));
}

M4 Camera::DelegateRH_ZO::perspective(float fov, float aspect, float near, float far)
{
    return M4(glm::perspectiveRH_ZO(fov, -aspect, near, far));
}

M4 Camera::DelegateLH_NO::frustum(float left, float right, float bottom, float top, float near, float far)
{
    return M4(glm::frustumLH_NO(left, right, bottom, top, near, far));
}

M4 Camera::DelegateLH_NO::lookAt(const V3& position, const V3& target, const V3& up)
{
    return M4(glm::lookAtLH(glm::vec3(position.x(), position.y(), position.z()), glm::vec3(target.x(), target.y(), target.z()), glm::vec3(up.x(), up.y(), up.z())));
}

M4 Camera::DelegateLH_NO::ortho(float left, float right, float bottom, float top, float near, float far)
{
    return M4(glm::orthoLH_NO(left, right, bottom, top, near, far));
}

M4 Camera::DelegateLH_NO::perspective(float fov, float aspect, float near, float far)
{
    return M4(glm::perspectiveLH_NO(fov, aspect, near, far));
}

M4 Camera::DelegateRH_NO::frustum(float left, float right, float bottom, float top, float near, float far)
{
    return M4(glm::frustumRH_NO(left, right, bottom, top, near, far));
}

M4 Camera::DelegateRH_NO::lookAt(const V3& position, const V3& target, const V3& up)
{
    return M4(glm::lookAtRH(glm::vec3(position.x(), position.y(), position.z()), glm::vec3(target.x(), target.y(), target.z()), glm::vec3(up.x(), up.y(), up.z())));
}

M4 Camera::DelegateRH_NO::ortho(float left, float right, float bottom, float top, float near, float far)
{
    return M4(glm::orthoRH_NO(left, right, bottom, top, near, far));
}

M4 Camera::DelegateRH_NO::perspective(float fov, float aspect, float near, float far)
{
    return M4(glm::perspectiveRH_NO(fov, aspect, near, far));
}

}
