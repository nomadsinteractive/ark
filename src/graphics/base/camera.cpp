#include "graphics/base/camera.h"

#include "core/base/observer.h"
#include "core/epi/notifier.h"

#include "core/impl/variable/variable_op2.h"
#include "core/types/global.h"
#include "core/util/operators.h"

namespace ark {

namespace {

class FrustumMatrixVariable : public Variable<Matrix> {
public:
    FrustumMatrixVariable(const sp<Vec3>& position, const sp<Vec3>& target, const sp<Vec3>& up, Notifier& notifier)
        : _position(position), _target(target), _up(up), _observer(notifier.createObserver()) {
    }

    virtual Matrix val() override {
        const V3 position = _position->val();
        const V3 target = _target->val();
        const V3 up = _up->val();
        if(_observer->dirty())
            _value = Matrix::lookAt(position, target, up);
        return _value;
    }

private:
    sp<Vec3> _position;
    sp<Vec3> _target;
    sp<Vec3> _up;
    Matrix _value;
    sp<Observer> _observer;
};

class MulMatrixVariable : public Variable<Matrix> {
public:
    MulMatrixVariable(const sp<Variable<Matrix>>& lvalue, const sp<Variable<Matrix>>& rvalue, Notifier& notifier)
        : _lvalue(lvalue), _rvalue(rvalue), _observer(notifier.createObserver()) {
    }

    virtual Matrix val() override {
        const Matrix lvalue = _lvalue->val();
        const Matrix rvalue = _rvalue->val();
        if(_observer->dirty())
            _value = lvalue * rvalue;
        return _value;
    }

private:
    sp<Variable<Matrix>> _lvalue;
    sp<Variable<Matrix>> _rvalue;
    Matrix _value;
    sp<Observer> _observer;
};

class V3DirtyChecker : public Vec3 {
public:
    V3DirtyChecker(const sp<Vec3>& delegate, const sp<Notifier>& notifier)
        : _delegate(delegate), _notifier(notifier), _value(delegate->val()) {
    }

    virtual V3 val() override {
        const V3 v = _delegate->val();
        if(_value != v) {
            _value = v;
            _notifier->notify();
        }
        return _value;
    }

private:
    sp<Vec3> _delegate;
    sp<Notifier> _notifier;
    V3 _value;
};

}

Camera::Camera()
    : _view(sp<Holder>::make(sp<Variable<Matrix>::Const>::make(Matrix()))), _projection(sp<Holder>::make(sp<Variable<Matrix>::Const>::make(Matrix()))),
      _notifier(sp<Notifier>::make())
{
}

void Camera::ortho(float left, float right, float top, float bottom, float near, float far)
{
    _vp = sp<Holder>::make(sp<Variable<Matrix>::Const>::make(Matrix::ortho(left, right, top, bottom, near * 2 - far, far)));
}

void Camera::perspective(float fov, float aspect, float near, float far)
{
    _projection = sp<Holder>::make(sp<Variable<Matrix>::Const>::make(Matrix::perspective(fov, aspect, near * 2 - far, far)));
    updateViewProjection();
}

void Camera::lookAt(const V3& position, const V3& target, const V3& up)
{
    _view = sp<Holder>::make(sp<Variable<Matrix>::Const>::make(Matrix::lookAt(position, target, up)));
    updateViewProjection();
}

void Camera::lookAt(const sp<Vec3>& position, const sp<Vec3>& target, const sp<Vec3>& up)
{
    _view = sp<Holder>::make(sp<FrustumMatrixVariable>::make(sp<V3DirtyChecker>::make(position, _notifier), sp<V3DirtyChecker>::make(target, _notifier),
                                                             sp<V3DirtyChecker>::make(up, _notifier), _notifier));
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

Camera::Snapshot Camera::snapshot() const
{
    return Snapshot(_vp);
}

const sp<Camera>& Camera::getMainCamera()
{
    const Global<Camera> camera;
    return camera;
}

void Camera::updateViewProjection()
{
    _vp = sp<Holder>::make(sp<MulMatrixVariable>::make(_projection->_delegate, _view->_delegate, _notifier));
}

Camera::Snapshot::Snapshot(Holder& holder)
{
    holder.flat(&_vp);
}

Camera::Holder::Holder(const sp<Variable<Matrix>>& delegate)
    : _delegate(delegate) {
}

void Camera::Holder::flat(void* buf)
{
    *reinterpret_cast<Matrix*>(buf) = _delegate->val();
}

uint32_t Camera::Holder::size()
{
    return sizeof(Matrix);
}

Matrix Camera::Holder::matrix()
{
    return _delegate->val();
}

}
