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
#include "graphics/util/matrix_util.h"
#include "graphics/util/mat4_type.h"
#include "graphics/util/vec3_type.h"

#include "renderer/base/render_engine.h"

#include "app/base/application_context.h"
#include "app/base/application_manifest.h"

namespace ark {

struct Camera::Stub {
    Stub()
        : _position(nullptr, V3(0, 0, 1)), _up(nullptr, V3(0, 1, 0)) {
    }

    OptionalVar<Vec3> _position;
    OptionalVar<Vec3> _target;
    OptionalVar<Vec3> _up;
};

namespace {

class OrthoMatrixVariable final : public Mat4 {
public:
    OrthoMatrixVariable(sp<Camera::Delegate> delegate, sp<Vec2> leftTop, sp<Vec2> rightBottom, sp<Vec2> clip)
        : _delegate(std::move(delegate)), _left_top(std::move(leftTop)), _right_bottom(std::move(rightBottom)), _clip(std::move(clip)),
          _matrix(calcMatrix()) {
    }

    M4 val() override {
        return _matrix;
    }

    bool update(const uint64_t timestamp) override {
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

class FrustumMatrixVariable final : public Mat4 {
public:
    FrustumMatrixVariable(sp<Camera::Delegate> delegate, sp<Camera::Stub> cameraStub)
        : _delegate(std::move(delegate)), _camera_stub(std::move(cameraStub)), _matrix(_delegate->lookAt(_camera_stub->_position.val(), _camera_stub->_target.val(), _camera_stub->_up.val()))
    {
    }

    M4 val() override
    {
        return _matrix;
    }

    bool update(const uint64_t timestamp) override {
        if(UpdatableUtil::update(timestamp, _camera_stub->_position, _camera_stub->_target, _camera_stub->_up))
        {
            _matrix = _delegate->lookAt(_camera_stub->_position.val(), _camera_stub->_target.val(), _camera_stub->_up.val());
            return true;
        }
        return false;
    }

private:
    sp<Camera::Delegate> _delegate;
    sp<Camera::Stub> _camera_stub;
    M4 _matrix;
};

class WorldPosition final : public Vec3 {
public:
    WorldPosition(sp<RenderEngine> renderEngine, sp<Mat4> viewProjectionInverse, sp<Vec3> screenPosition)
        : _render_engine(std::move(renderEngine)), _view_projecttion_inverse(std::move(viewProjectionInverse)), _screen_position(std::move(screenPosition))
    {
    }

    bool update(const uint64_t timestamp) override
    {
        return UpdatableUtil::update(timestamp, _view_projecttion_inverse, _screen_position);
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

constexpr char sclipNearclipFarPlaneWarning[] = "ClipNear: %.2f, ClipFar: %.2f. Far plane should be further than near plane, and distance to the near plane should be greater than zero.";

}

Camera::Camera(const enums::CoordinateSystem coordinateSystem)
    : Camera(Ark::instance().createCamera(coordinateSystem))
{
}

Camera::Camera(const Camera& other)
    : Camera(other._coordinate_system, other._delegate, other._view, other._projection)
{
    _stub->_position.reset(sp<Vec3>::make<Vec3::Const>(other._stub->_position.val()));
    _stub->_target.reset(sp<Vec3>::make<Vec3::Const>(other._stub->_target.val()));
    _stub->_up.reset(sp<Vec3>::make<Vec3::Const>(other._stub->_up.val()));
}

Camera::Camera(const enums::CoordinateSystem coordinateSystem, sp<Delegate> delegate, sp<Mat4> view, sp<Mat4> projection)
    : _coordinate_system(coordinateSystem), _delegate(std::move(delegate)), _view(sp<Mat4>::make<Mat4Wrapper>(view ? std::move(view) : Mat4Type::create())), _projection(sp<Mat4>::make<Mat4Wrapper>(projection ? std::move(projection) : Mat4Type::create())),
      _vp(sp<Mat4>::make<Mat4Wrapper>(Mat4Type::matmul(_projection.cast<Mat4>(), _view.cast<Mat4>()))), _stub(sp<Stub>::make())
{
    CHECK(_coordinate_system == enums::COORDINATE_SYSTEM_LHS || _coordinate_system == enums::COORDINATE_SYSTEM_RHS, "Camera's coordinate system should be either LHS or RHS");
}

void Camera::ortho(const V2& leftTop, const V2& rightBottom, const V2& clip)
{
    ortho(leftTop.x(), rightBottom.x(), rightBottom.y(), leftTop.y(), clip.x(), clip.y());
}

void Camera::ortho(sp<Vec2> leftTop, sp<Vec2> rightBottom, sp<Vec2> clip)
{
    _projection->set(sp<Mat4>::make<OrthoMatrixVariable>(_delegate, std::move(leftTop), std::move(rightBottom), std::move(clip)));
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

void Camera::perspective(const float fov, const float aspect, const float clipNear, const float clipFar)
{
    CHECK_WARN(clipFar > clipNear && clipNear > 0, sclipNearclipFarPlaneWarning, clipNear, clipFar);
    _projection->set(_delegate->perspective(fov, aspect, clipNear, clipFar));
}

void Camera::lookAt(const V3& position, const V3& target, const V3& up)
{
    _stub->_position.reset(sp<Vec3>::make<Vec3::Const>(position));
    _stub->_target.reset(sp<Vec3>::make<Vec3::Const>(target));
    _stub->_up.reset(sp<Vec3>::make<Vec3::Const>(up));
    _view->set(_delegate->lookAt(position, target, up));
}

sp<Vec3> Camera::toWorldPosition(sp<Vec3> screenPosition) const
{
    sp<Mat4> vpInverse = Mat4Type::inverse(_vp);
    return sp<Vec3>::make<WorldPosition>(Ark::instance().applicationContext()->renderEngine(), std::move(vpInverse), std::move(screenPosition));
}

void Camera::lookAt(sp<Vec3> position, sp<Vec3> target, sp<Vec3> up)
{
    _stub->_position.reset(std::move(position));
    _stub->_target.reset(std::move(target));
    _stub->_up.reset(std::move(up));
    _view->set(sp<Mat4>::make<FrustumMatrixVariable>(_delegate, _stub));
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
    return _stub->_position.toVar();
}

sp<Vec3> Camera::target() const
{
    return _stub->_target.toVar();
}

sp<Vec3> Camera::up() const
{
    return _stub->_up.toVar();
}

void Camera::assign(const Camera& other)
{
    _coordinate_system = other._coordinate_system;
    _delegate = other._delegate;
    _stub->_position.reset(other._stub->_position.toVar());
    _stub->_target.reset(other._stub->_target.toVar());
    _stub->_up.reset(other._stub->_up.toVar());
    _view->set(other.view());
    _projection->set(other.projection());
}

sp<Mat4> Camera::view() const
{
    return _view;
}

void Camera::setView(sp<Mat4> view)
{
    _view->set(std::move(view));
}

sp<Mat4> Camera::projection() const
{
    return _projection;
}

void Camera::setProjection(sp<Mat4> projection)
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

M4 Camera::DelegateLH_ZO::frustum(float left, float right, float bottom, float top, float clipNear, float clipFar)
{
    return {glm::frustumLH_ZO(left, right, bottom, top, clipNear, clipFar)};
}

M4 Camera::DelegateLH_ZO::lookAt(const V3& position, const V3& target, const V3& up)
{
    return {glm::lookAtLH(glm::vec3(position.x(), position.y(), position.z()), glm::vec3(target.x(), target.y(), target.z()), glm::vec3(up.x(), up.y(), up.z()))};
}

M4 Camera::DelegateLH_ZO::ortho(float left, float right, float bottom, float top, float clipNear, float clipFar)
{
    return {glm::orthoLH_ZO(left, right, bottom, top, clipNear, clipFar)};
}

M4 Camera::DelegateLH_ZO::perspective(float fov, float aspect, float clipNear, float clipFar)
{
    return {glm::perspectiveLH_ZO(fov, aspect, clipNear, clipFar)};
}

M4 Camera::DelegateRH_ZO::frustum(float left, float right, float bottom, float top, float clipNear, float clipFar)
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
