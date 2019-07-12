#include "graphics/base/camera.h"

#include "core/ark.h"

#include "core/base/observer.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/epi/notifier.h"
#include "core/impl/boolean/boolean_by_weak_ref.h"
#include "core/impl/variable/variable_op2.h"
#include "core/types/global.h"
#include "core/util/operators.h"

#include "app/base/application_context.h"

namespace ark {

namespace {

class FrustumMatrixVariable : public Variable<Matrix>, public Runnable {
public:
    FrustumMatrixVariable(const sp<Vec3>& position, const sp<Vec3>& target, const sp<Vec3>& up, const sp<Notifier>& notifier)
        : _position(position), _target(target), _up(up), _notifier(notifier) {
    }

    virtual void run() override {
        update();
    }

    virtual Matrix val() override {
        return _matrix;
    }

private:
    void update() {
        const V3 position = _position->val();
        const V3 target = _target->val();
        const V3 up = _up->val();
        if(_p != position || _t != target || _u != up) {
            _matrix = Matrix::lookAt(position, target, up);
            _notifier->notify();
            _p = position;
            _t = target;
            _u = up;
        }
    }

private:
    sp<Vec3> _position;
    sp<Vec3> _target;
    sp<Vec3> _up;

    sp<Notifier> _notifier;

    V3 _p;
    V3 _t;
    V3 _u;

    Matrix _matrix;
};

class MulMatrixVariable : public Variable<Matrix> {
public:
    MulMatrixVariable(const sp<Variable<Matrix>>& lvalue, const sp<Variable<Matrix>>& rvalue)
        : _lvalue(lvalue), _rvalue(rvalue) {
    }

    virtual Matrix val() override {
        const Matrix lvalue = _lvalue->val();
        const Matrix rvalue = _rvalue->val();
        return lvalue * rvalue;
    }

private:
    sp<Variable<Matrix>> _lvalue;
    sp<Variable<Matrix>> _rvalue;
};

}

Camera::Camera()
    : _view(sp<Holder>::make(sp<Variable<Matrix>::Const>::make(Matrix()))), _projection(sp<Holder>::make(sp<Variable<Matrix>::Const>::make(Matrix()))),
      _vp(sp<Holder>::make(sp<Variable<Matrix>::Const>::make(Matrix()))), _notifier(sp<Notifier>::make())
{
}

void Camera::ortho(float left, float right, float bottom, float top, float near, float far, float upDirection)
{
    if(upDirection  < 0)
        std::swap(top, bottom);

    _vp->_value = sp<Variable<Matrix>::Const>::make(Matrix::ortho(left, right, bottom, top, near * 2 - far, far));
    _notifier->notify();
}

void Camera::frustum(float left, float right, float bottom, float top, float near, float far)
{
    _projection->_value = sp<Variable<Matrix>::Const>::make(Matrix::frustum(left, right, bottom, top, near, far));
    updateViewProjection();
}

void Camera::perspective(float fov, float aspect, float near, float far)
{
    _projection->_value = sp<Variable<Matrix>::Const>::make(Matrix::perspective(fov, aspect, near, far));
    updateViewProjection();
}

void Camera::lookAt(const V3& position, const V3& target, const V3& up)
{
    _view->_value = sp<Variable<Matrix>::Const>::make(Matrix::lookAt(position, target, up));
    updateViewProjection();
}

void Camera::lookAt(const sp<Vec3>& position, const sp<Vec3>& target, const sp<Vec3>& up)
{
    const sp<FrustumMatrixVariable> var = sp<FrustumMatrixVariable>::make(position, target, up, _notifier);
    _view->_value = var;
    Ark::instance().applicationContext()->addPreRenderTask(var, sp<BooleanByWeakRef<Runnable>>::make(var, 1));
    updateViewProjection();
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

const sp<Notifier>& Camera::notifier() const
{
    return _notifier;
}

const sp<Camera>& Camera::getDefaultCamera()
{
    const Global<Camera> camera;
    DCHECK(camera->vp(), "Default camera has not been uninitialized");
    return camera;
}

void Camera::updateViewProjection()
{
    _vp->_value = sp<MulMatrixVariable>::make(_projection->_value, _view->_value);
    _notifier->notify();
}

Camera::Holder::Holder(const sp<Variable<Matrix>>& value)
    : _value(value) {
}

void Camera::Holder::flat(void* buf)
{
    *reinterpret_cast<Matrix*>(buf) = _value->val();
}

uint32_t Camera::Holder::size()
{
    return sizeof(Matrix);
}

}
