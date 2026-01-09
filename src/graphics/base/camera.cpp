#include "graphics/base/camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "core/ark.h"

#include "core/impl/variable/variable_wrapper.h"
#include "core/inf/writable.h"
#include "core/types/global.h"
#include "core/util/updatable_util.h"

#include "graphics/base/mat.h"
#include "graphics/base/viewport.h"
#include "graphics/components/translation.h"
#include "graphics/util/matrix_util.h"
#include "graphics/util/mat4_type.h"
#include "graphics/util/vec3_type.h"

#include "renderer/base/render_engine.h"

#include "app/base/application_context.h"
#include "app/base/application_manifest.h"

namespace ark {

struct PerspectiveStub {
    PerspectiveStub(sp<Numeric> fov, sp<Numeric> aspect, sp<Numeric> clipNear, sp<Numeric> clipFar)
        : _fov(sp<VariableWrapper<float>>::make(std::move(fov))), _aspect(sp<VariableWrapper<float>>::make(std::move(aspect))), _clip_near(sp<VariableWrapper<float>>::make(std::move(clipNear))), _clip_far(sp<VariableWrapper<float>>::make(std::move(clipFar))) {
    }

    sp<VariableWrapper<float>> _fov;
    sp<VariableWrapper<float>> _aspect;
    sp<VariableWrapper<float>> _clip_near;
    sp<VariableWrapper<float>> _clip_far;
};

struct LookAtStub {
    LookAtStub(sp<Vec3> position, sp<Vec3> target, sp<Vec3> up)
        : _position(sp<VariableWrapper<V3>>::make(std::move(position))), _target(sp<VariableWrapper<V3>>::make(std::move(target))), _up(sp<VariableWrapper<V3>>::make(std::move(up))) {
    }

    sp<VariableWrapper<V3>> _position;
    sp<VariableWrapper<V3>> _target;
    sp<VariableWrapper<V3>> _up;
};

struct Camera::Stub {
    Optional<PerspectiveStub> _perspective;
    Optional<LookAtStub> _look_at;

    Stub() = default;
    Stub(const Stub& other)
    {
        if(const Optional<PerspectiveStub>& opt = other._perspective)
            _perspective = {{opt->_fov->wrapped(), opt->_aspect->wrapped(), opt->_clip_near->wrapped(), opt->_clip_far->wrapped()}};

        if(const Optional<LookAtStub>& opt = other._look_at)
            _look_at = {{opt->_position->wrapped(), opt->_target->wrapped(), opt->_up->wrapped()}};
    }

    void perspective(sp<Numeric> fov, sp<Numeric> aspect, sp<Numeric> clipNear, sp<Numeric> clipFar)
    {
        if(_perspective)
        {
            _perspective->_fov->reset(std::move(fov));
            _perspective->_aspect->reset(std::move(aspect));
            _perspective->_clip_near->reset(std::move(clipNear));
            _perspective->_clip_far->reset(std::move(clipFar));
        }
        else
        {
            _perspective = {{std::move(fov), std::move(aspect), std::move(clipNear), std::move(clipFar)}};
        }
    }

    void lookAt(sp<Vec3> position, sp<Vec3> target, sp<Vec3> up)
    {
        if(_look_at)
        {
            _look_at->_position->reset(std::move(position));
            _look_at->_target->reset(std::move(target));
            _look_at->_up->reset(std::move(up));
        }
        else
        {
            _look_at = {{std::move(position), std::move(target), std::move(up)}};
        }
    }
};

namespace {

class OrthoMat4 final : public Mat4 {
public:
    OrthoMat4(sp<Camera::Delegate> delegate, sp<Vec2> leftTop, sp<Vec2> rightBottom, sp<Vec2> clip)
        : _delegate(std::move(delegate)), _left_top(std::move(leftTop)), _right_bottom(std::move(rightBottom)), _clip(std::move(clip)),
          _matrix(calcMatrix()) {
    }

    M4 val() override {
        return _matrix;
    }

    bool update(uint32_t tick) override {
        if(UpdatableUtil::update(tick, _left_top, _right_bottom, _clip)) {
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

class PerspectiveMat4 final : public Mat4 {
public:
    PerspectiveMat4(const sp<Camera::Delegate>& delegate, const sp<Camera::Stub>& camera)
        : _delegate(delegate), _camera(camera) {
        update(Timestamp::now());
    }

    M4 val() override
    {
        return _matrix;
    }

    bool update(uint32_t tick) override
    {
        if(const Optional<PerspectiveStub>& perspective = _camera->_perspective; UpdatableUtil::update(tick, perspective->_fov, perspective->_aspect, perspective->_clip_near, perspective->_clip_far))
        {
            _matrix = _delegate->perspective(perspective->_fov->val(), perspective->_aspect->val(), perspective->_clip_near->val(), perspective->_clip_far->val());
            return true;
        }
        return false;
    }

private:
    sp<Camera::Delegate> _delegate;
    sp<Camera::Stub> _camera;
    M4 _matrix;
};

class LookAtMat4 final : public Mat4 {
public:
    LookAtMat4(const sp<Camera::Delegate>& delegate, const sp<Camera::Stub>& camera)
        : _delegate(delegate), _camera(camera) {
        update(Timestamp::now());
    }

    M4 val() override
    {
        return _matrix;
    }

    bool update(uint32_t tick) override
    {
        if(const Optional<LookAtStub>& lookAt = _camera->_look_at; lookAt && UpdatableUtil::update(tick, lookAt->_position, lookAt->_target, lookAt->_up))
        {
            _matrix = _delegate->lookAt(lookAt->_position->val(), lookAt->_target->val(), lookAt->_up->val());
            return true;
        }
        return false;
    }

private:
    sp<Camera::Delegate> _delegate;
    sp<Camera::Stub> _camera;
    M4 _matrix;
};

class WorldPosition final : public Vec3 {
public:
    WorldPosition(sp<RenderEngine> renderEngine, sp<Mat4> viewProjectionInverse, sp<Vec3> screenPosition)
        : _render_engine(std::move(renderEngine)), _view_projecttion_inverse(std::move(viewProjectionInverse)), _screen_position(std::move(screenPosition))
    {
    }

    bool update(const uint32_t tick) override
    {
        return UpdatableUtil::update(tick, _view_projecttion_inverse, _screen_position);
    }

    V3 val() override
    {
        const V3 pos = _screen_position->val();
        const V2 ndc = _render_engine->toNDC(pos.x(), pos.y());
        return MatrixUtil::mul(_view_projecttion_inverse->val(), V3(ndc, pos.z()));
    }

private:
    sp<RenderEngine> _render_engine;
    sp<Mat4> _view_projecttion_inverse;
    sp<Vec3> _screen_position;
};

class RUF2Quaternion final : public Vec4 {
public:
    RUF2Quaternion(sp<Vec3> right, sp<Vec3> up, sp<Vec3> front)
        : _right(std::move(right)), _up(std::move(up)), _front(std::move(front))
    {
        update(Timestamp::now());
    }

    bool update(uint32_t tick) override
    {
        if(UpdatableUtil::update(tick, _front, _right, _up))
        {
            _quaternion = doUpdate();
            return true;
        }
        return false;
    }

    V4 val() override
    {
        return _quaternion;
    }

private:

    V4 doUpdate() const
    {
        const V3 F = _front->val();
        const V3 R = _right->val();
        const V3 U = _up->val();
        if(const float trace = R.x() + U.y() + F.z(); trace > 0)
        {
            const double s = 0.5f / std::sqrt(trace + 1.0f);
            return V4((U.z() - F.y()) * s, (F.x() - R.z()) * s, (R.y() - U.x()) * s, 0.25f / s);
        }
        if(R.x() > U.y() && R.x() > F.z())
        {
            const double s = 2.0f * std::sqrt(1.0f + R.x() - U.y() - F.z());
            return V4(0.25f * s, (U.x() + R.y()) / s, (F.x() + R.z()) / s, (U.z() - F.y()) / s);
        }
        if(U.y() > F.z())
        {
            const double s = 2.0f * sqrt(1.0f + U.y() - R.x() - F.z());
            return V4((U.x() + R.y()) / s, 0.25f * s, (F.y() + U.z()) / s, (F.x() - R.z()) / s);
        }
        const double s = 2.0 * sqrt(1.0f + F.z() - R.x() - U.y());
        return V4((F.x() + R.z()) / s, (F.y() + U.z()) / s, 0.25f * s, (R.y() - U.x()) / s);
    }

private:
    sp<Vec3> _right;
    sp<Vec3> _up;
    sp<Vec3> _front;
    V4 _quaternion;
};

constexpr char sclipNearclipFarPlaneWarning[] = "ClipNear: %.2f, ClipFar: %.2f. Far plane should be further than near plane, and distance to the near plane should be greater than zero.";

}

Camera::Camera(const enums::CoordinateSystem coordinateSystem)
    : Camera(Ark::instance().createCamera(coordinateSystem))
{
}

Camera::Camera(const Camera& other)
    : Camera(other._coordinate_system, other._delegate, other._view, other._projection, sp<Stub>::make(*other._stub))
{
}

Camera::Camera(const enums::CoordinateSystem coordinateSystem, sp<Delegate> delegate, sp<Mat4> view, sp<Mat4> projection, sp<Stub> stub)
    : _coordinate_system(coordinateSystem), _delegate(std::move(delegate)), _view(sp<Mat4>::make<Mat4Wrapper>(view ? std::move(view) : Mat4Type::create())), _projection(sp<Mat4>::make<Mat4Wrapper>(projection ? std::move(projection) : Mat4Type::create())),
      _vp(sp<Mat4>::make<Mat4Wrapper>(Mat4Type::matmul(_projection.cast<Mat4>(), _view.cast<Mat4>()))), _stub(stub ? std::move(stub) : sp<Stub>::make())
{
    CHECK(_coordinate_system == enums::COORDINATE_SYSTEM_LHS || _coordinate_system == enums::COORDINATE_SYSTEM_RHS, "Camera's coordinate system should be either LHS or RHS");
}

enums::CoordinateSystem Camera::coordinateSystem() const
{
    return _coordinate_system;
}

void Camera::ortho(const V2& leftTop, const V2& rightBottom, const V2& clip)
{
    ortho(leftTop.x(), rightBottom.x(), rightBottom.y(), leftTop.y(), clip.x(), clip.y());
}

void Camera::ortho(sp<Vec2> leftTop, sp<Vec2> rightBottom, sp<Vec2> clip)
{
    _projection->set(sp<Mat4>::make<OrthoMat4>(_delegate, std::move(leftTop), std::move(rightBottom), std::move(clip)));
}

void Camera::ortho(const float left, const float right, const float bottom, const float top, const float clipNear, const float clipFar)
{
    _projection->set(_delegate->ortho(left, right, bottom, top, clipNear, clipFar));
}

void Camera::frustum(const float left, const float right, const float bottom, const float top, const float clipNear, const float clipFar)
{
    CHECK_WARN(clipFar > clipNear && clipNear > 0, sclipNearclipFarPlaneWarning, clipNear, clipFar);
    _projection->set(_delegate->frustum(left, right, bottom, top, clipNear, clipFar));
}

void Camera::perspective(const float fov, const float aspect, const float clipNear, const float clipFar) const
{
    CHECK_WARN(clipFar > clipNear && clipNear > 0, sclipNearclipFarPlaneWarning, clipNear, clipFar);
    _stub->perspective(sp<Numeric>::make<Numeric::Const>(fov), sp<Numeric>::make<Numeric::Const>(aspect), sp<Numeric>::make<Numeric::Const>(clipNear), sp<Numeric>::make<Numeric::Const>(clipFar));
    _projection->set(_delegate->perspective(fov, aspect, clipNear, clipFar));
}

void Camera::perspective(sp<Numeric> fov, sp<Numeric> aspect, sp<Numeric> clipNear, sp<Numeric> clipFar) const
{
    _stub->perspective(std::move(fov), std::move(aspect), std::move(clipNear), std::move(clipFar));
    _projection->set(sp<Mat4>::make<PerspectiveMat4>(_delegate, _stub));
}

sp<Vec3> Camera::toWorldPosition(sp<Vec3> screenPosition) const
{
    sp<Mat4> vpInverse = Mat4Type::inverse(_vp);
    return sp<Vec3>::make<WorldPosition>(Ark::instance().applicationContext()->renderEngine(), std::move(vpInverse), std::move(screenPosition));
}

void Camera::lookAt(const V3 position, const V3 target, const V3 up) const
{
    _stub->lookAt(sp<Vec3>::make<Vec3::Const>(position), sp<Vec3>::make<Vec3::Const>(target), sp<Vec3>::make<Vec3::Const>(up));
    _view->set(_delegate->lookAt(position, target, up));
}

void Camera::lookAt(sp<Vec3> position, sp<Vec3> target, sp<Vec3> up) const
{
    _stub->lookAt(std::move(position), std::move(target), std::move(up));
    _view->set(sp<Mat4>::make<LookAtMat4>(_delegate, _stub));
}

V3 Camera::toViewportPosition(const V3& worldPosition) const
{
    return MatrixUtil::mul(_vp->val(), worldPosition);
}

sp<Vec3> Camera::toViewportPosition(sp<Vec3> worldPosition) const
{
    const Viewport& viewport = Ark::instance().applicationContext()->renderEngine()->viewport();
    const V3 scale = Ark::instance().manifest()->renderer()._coordinate_system == _coordinate_system ? V3(0.5f, 0.5f, 0.5f) : V3(0.5f, -0.5f, 0.5f);
    return Vec3Type::mul(Vec3Type::add(Vec3Type::mul(Mat4Type::matmul(_vp, std::move(worldPosition)), scale), V3(0.5f, 0.5f, 0.5f)), V3(viewport.width(), viewport.height(), 1.0f));
}

sp<Vec3> Camera::position() const
{
    return _stub->_look_at ? _stub->_look_at->_position : nullptr;
}

sp<Vec3> Camera::target() const
{
    return _stub->_look_at ? _stub->_look_at->_target : nullptr;
}

sp<Vec3> Camera::up() const
{
    return _stub->_look_at ? _stub->_look_at->_up : nullptr;
}

sp<Numeric> Camera::fov() const
{
    return _stub->_perspective ? _stub->_perspective->_fov : nullptr;
}

sp<Numeric> Camera::aspect() const
{
    return _stub->_perspective ? _stub->_perspective->_aspect : nullptr;
}

sp<Numeric> Camera::clipNear() const
{
    return _stub->_perspective ? _stub->_perspective->_clip_near : nullptr;
}

sp<Numeric> Camera::clipFar() const
{
    return _stub->_perspective ? _stub->_perspective->_clip_far : nullptr;
}

void Camera::assign(const Camera& other)
{
    _coordinate_system = other._coordinate_system;
    _delegate = other._delegate;
    if(const Optional<PerspectiveStub>& opt = other._stub->_perspective)
        perspective(opt->_fov->wrapped(), opt->_aspect->wrapped(), opt->_clip_near->wrapped(), opt->_clip_far->wrapped());
    else
        _projection->set(other.projection());
    if(const Optional<LookAtStub>& opt = other._stub->_look_at)
        lookAt(opt->_position->wrapped(), opt->_target->wrapped(), opt->_up->wrapped());
    else
        _view->set(other.view());
}

sp<Mat4> Camera::toFrustumSliceMatrix(const float z, const bool normalize) const
{
    auto [center, matrix] = toFrustumSliceCenterAndMatrix(z, normalize);
    return Mat4Type::matmul(sp<Translation>::make(std::move(center))->toMatrix(), std::move(matrix));
}

std::pair<sp<Vec3>, sp<Mat4>> Camera::toFrustumSliceCenterAndMatrix(const float z, const bool normalize) const
{
    sp<Mat4> vpInverse = Mat4Type::inverse(_vp);
    sp<Vec3> position = Mat4Type::matmul(vpInverse, V3(0, 0, z));
    sp<Vec3> right = Vec3Type::sub(Mat4Type::matmul(vpInverse, V3(1, 0, z)), position);
    sp<Vec3> up = Vec3Type::sub(Mat4Type::matmul(std::move(vpInverse), V3(0, 1, z)), position);
    sp<Vec3> front = sp<Vec3>::make<Vec3::Const>(V3(0));
    if(normalize)
    {
        right = Vec3Type::normalize(std::move(right));
        up = Vec3Type::normalize(std::move(up));
    }
    const Constants& constants = Global<Constants>();
    sp<Mat4> matrix = Mat4Type::create(Vec3Type::extend(std::move(right), constants.NUMERIC_ZERO), Vec3Type::extend(std::move(up), constants.NUMERIC_ZERO), Vec3Type::extend(std::move(front), constants.NUMERIC_ZERO), sp<Vec4>::make<Vec4::Const>(V4(0, 0, 0, 1)));
    return {std::move(position), std::move(matrix)};
}

sp<Mat4> Camera::view() const
{
    return _view;
}

void Camera::setView(sp<Mat4> view) const
{
    _view->set(std::move(view));
}

sp<Mat4> Camera::projection() const
{
    return _projection;
}

void Camera::setProjection(sp<Mat4> projection) const
{
    _projection->set(std::move(projection));
}

sp<Mat4> Camera::vp() const
{
    return _vp;
}

bool Camera::isLHS() const
{
    return _coordinate_system == enums::COORDINATE_SYSTEM_LHS;
}

Camera Camera::createDefaultCamera()
{
    const Camera& defaultCamera = Global<Camera>();
    return defaultCamera;
}

M4 Camera::DelegateLH_ZO::frustum(const float left, const float right, const float bottom, const float top, const float clipNear, const float clipFar)
{
    return {glm::frustumLH_ZO(left, right, bottom, top, clipNear, clipFar)};
}

M4 Camera::DelegateLH_ZO::lookAt(const V3& position, const V3& target, const V3& up)
{
    return {glm::lookAtLH(glm::vec3(position.x(), position.y(), position.z()), glm::vec3(target.x(), target.y(), target.z()), glm::vec3(up.x(), up.y(), up.z()))};
}

M4 Camera::DelegateLH_ZO::ortho(const float left, const float right, const float bottom, const float top, const float clipNear, const float clipFar)
{
    return {glm::orthoLH_ZO(left, right, bottom, top, clipNear, clipFar)};
}

M4 Camera::DelegateLH_ZO::perspective(const float fov, const float aspect, const float clipNear, const float clipFar)
{
    return {glm::perspectiveLH_ZO(fov, aspect, clipNear, clipFar)};
}

M4 Camera::DelegateRH_ZO::frustum(const float left, const float right, const float bottom, const float top, const float clipNear, const float clipFar)
{
    return {glm::frustumRH_ZO(left, right, bottom, top, clipNear, clipFar)};
}

M4 Camera::DelegateRH_ZO::lookAt(const V3& position, const V3& target, const V3& up)
{
    return {lookAtRH(glm::vec3(position.x(), position.y(), position.z()), glm::vec3(target.x(), target.y(), target.z()), glm::vec3(up.x(), up.y(), up.z()))};
}

M4 Camera::DelegateRH_ZO::ortho(const float left, const float right, const float bottom, const float top, const float clipNear, const float clipFar)
{
    return {glm::orthoRH_ZO(left, right, bottom, top, clipNear, clipFar)};
}

M4 Camera::DelegateRH_ZO::perspective(const float fov, const float aspect, const float clipNear, const float clipFar)
{
    return {glm::perspectiveRH_ZO(fov, aspect, clipNear, clipFar)};
}

M4 Camera::DelegateLH_NO::frustum(const float left, const float right, const float bottom, const float top, const float clipNear, const float clipFar)
{
    return {glm::frustumLH_NO(left, right, bottom, top, clipNear, clipFar)};
}

M4 Camera::DelegateLH_NO::lookAt(const V3& position, const V3& target, const V3& up)
{
    return {glm::lookAtLH(glm::vec3(position.x(), position.y(), position.z()), glm::vec3(target.x(), target.y(), target.z()), glm::vec3(up.x(), up.y(), up.z()))};
}

M4 Camera::DelegateLH_NO::ortho(const float left, const float right, const float bottom, const float top, const float clipNear, const float clipFar)
{
    return {glm::orthoLH_NO(left, right, bottom, top, clipNear, clipFar)};
}

M4 Camera::DelegateLH_NO::perspective(const float fov, const float aspect, const float clipNear, const float clipFar)
{
    return {glm::perspectiveLH_NO(fov, aspect, clipNear, clipFar)};
}

M4 Camera::DelegateRH_NO::frustum(const float left, const float right, const float bottom, const float top, const float clipNear, const float clipFar)
{
    return {glm::frustumRH_NO(left, right, bottom, top, clipNear, clipFar)};
}

M4 Camera::DelegateRH_NO::lookAt(const V3& position, const V3& target, const V3& up)
{
    return {glm::lookAtRH(glm::vec3(position.x(), position.y(), position.z()), glm::vec3(target.x(), target.y(), target.z()), glm::vec3(up.x(), up.y(), up.z()))};
}

M4 Camera::DelegateRH_NO::ortho(const float left, const float right, const float bottom, const float top, const float clipNear, const float clipFar)
{
    return {glm::orthoRH_NO(left, right, bottom, top, clipNear, clipFar)};
}

M4 Camera::DelegateRH_NO::perspective(const float fov, const float aspect, const float clipNear, const float clipFar)
{
    return {glm::perspectiveRH_NO(fov, aspect, clipNear, clipFar)};
}

}
