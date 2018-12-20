#include "graphics/base/camera.h"

#include "core/impl/boolean/boolean_or.h"
#include "core/impl/variable/variable_op2.h"
#include "core/types/global.h"
#include "core/util/operators.h"
#include "core/util/log.h"

namespace ark {

namespace {

class FrustumMatrixVariable : public Variable<Matrix> {
public:
    FrustumMatrixVariable(const sp<Vec3>& position, const sp<Vec3>& target, const sp<Vec3>& up)
        : _position(position), _target(target), _up(up) {
    }

    virtual Matrix val() override {
        return Matrix::lookAt(_position->val(), _target->val(), _up->val());
    }

private:
    sp<Vec3> _position;
    sp<Vec3> _target;
    sp<Vec3> _up;
};

class V3DirtyChecker : public Boolean {
public:
    V3DirtyChecker(const sp<Vec3>& delegate)
        : _delegate(delegate), _dirty(false), _value(delegate->val()) {
    }

    virtual bool val() override {
        const V3 v = _delegate->val();
        _dirty = _dirty || v != _value;
        _value = v;
        return _dirty;
    }

    void reset() {
        _dirty = false;
    }

private:
    sp<Vec3> _delegate;
    bool _dirty;
    V3 _value;
};

class DirtyChecker : public Boolean {
public:
    DirtyChecker(const sp<Changed>& dirty, const sp<V3DirtyChecker>& a1, const sp<V3DirtyChecker>& a2, const sp<V3DirtyChecker>& a3)
        : _dirty(dirty), _a1(a1), _a2(a2), _a3(a3) {
    }

    virtual bool val() override {
        return isDirty(_a1) || isDirty(_a2) || isDirty(_a3);
    }

private:
    bool isDirty(V3DirtyChecker& a) {
        bool dirty = a.val();
        if(dirty) {
            _dirty->notify();
            a.reset();
        }
        return dirty;
    }

private:
    sp<Changed> _dirty;
    sp<V3DirtyChecker> _a1;
    sp<V3DirtyChecker> _a2;
    sp<V3DirtyChecker> _a3;
};

}

Camera::Camera()
    : _view(sp<Holder>::make(sp<Variable<Matrix>::Const>::make(Matrix()), sp<Boolean::Const>::make(false))),
      _projection(sp<Holder>::make(sp<Variable<Matrix>::Const>::make(Matrix()), sp<Boolean::Const>::make(false))),
      _dirty(sp<Changed>::make(false))
{
}

void Camera::ortho(float left, float right, float top, float bottom, float near, float far)
{
    _vp = sp<Holder>::make(sp<Variable<Matrix>::Const>::make(Matrix::ortho(left, right, top, bottom, near, far)), sp<Boolean::Const>::make(false));
}

void Camera::perspective(float fov, float aspect, float near, float far)
{
    _projection = sp<Holder>::make(sp<Variable<Matrix>::Const>::make(Matrix::perspective(fov, aspect, near, far)), sp<Boolean::Const>::make(false));
    updateViewProjection();
}

void Camera::lookAt(const V3& position, const V3& target, const V3& up)
{
    _position = sp<Variable<V3>::Const>::make(position);
    _target = sp<Variable<V3>::Const>::make(target);
    _up = sp<Variable<V3>::Const>::make(up);

    _view = sp<Holder>::make(sp<Variable<Matrix>::Const>::make(Matrix::lookAt(position, target, up)), sp<Boolean::Const>::make(false));
    updateViewProjection();
}

void Camera::lookAt(const sp<Vec3>& position, const sp<Vec3>& target, const sp<Vec3>& up)
{
    _position = position;
    _target = target;
    _up = up;

    _view = sp<Holder>::make(sp<FrustumMatrixVariable>::make(position, target, up), sp<DirtyChecker>::make(_dirty, sp<V3DirtyChecker>::make(position), sp<V3DirtyChecker>::make(target), sp<V3DirtyChecker>::make(up)));
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
    _vp = sp<Holder>::make(sp<VariableOP2<Matrix, Matrix, Operators::Mul<Matrix>, sp<Variable<Matrix>>,
                           sp<Variable<Matrix>>>>::make(_projection->_delegate, _view->_delegate), _dirty->toBoolean());
}

Camera::Snapshot::Snapshot(Holder& holder)
{
    holder.flat(&_vp);
}

Camera::Holder::Holder(const sp<Variable<Matrix>>& delegate, const sp<Boolean>& dirty)
    : _delegate(delegate), _notifier(sp<Changed>::make(dirty)), _value(delegate->val()) {
}

void Camera::Holder::flat(void* buf)
{
    _value = _delegate->val();
    *reinterpret_cast<Matrix*>(buf) = _value;
}

uint32_t Camera::Holder::size()
{
    return sizeof(Matrix);
}

uint32_t Camera::Holder::length()
{
    return 1;
}

Matrix Camera::Holder::matrix()
{
    if(_notifier->hasChanged())
        _value = _delegate->val();
    return _value;
}

}
