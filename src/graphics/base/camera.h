#pragma once

#include "core/ark.h"
#include "core/forwarding.h"
#include "core/inf/uploader.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

namespace ark {

class ARK_API Camera {
public:
    class Delegate {
    public:
        virtual ~Delegate() = default;

        virtual M4 frustum(float left, float right, float bottom, float top, float clipNear, float clipFar) = 0;
        virtual M4 lookAt(const V3& position, const V3& target, const V3& up) = 0;
        virtual M4 ortho(float left, float right, float bottom, float top, float clipNear, float clipFar) = 0;
        virtual M4 perspective(float fov, float aspect, float clipNear, float clipFar) = 0;
    };

    class DelegateLH_ZO final : public Delegate {
    public:
        M4 frustum(float left, float right, float bottom, float top, float clipNear, float clipFar) override;
        M4 lookAt(const V3& position, const V3& target, const V3& up) override;
        M4 ortho(float left, float right, float bottom, float top, float clipNear, float clipFar) override;
        M4 perspective(float fov, float aspect, float clipNear, float clipFar) override;
    };

    class DelegateRH_ZO final : public Delegate {
    public:
        M4 frustum(float left, float right, float bottom, float top, float clipNear, float clipFar) override;
        M4 lookAt(const V3& position, const V3& target, const V3& up) override;
        M4 ortho(float left, float right, float bottom, float top, float clipNear, float clipFar) override;
        M4 perspective(float fov, float aspect, float clipNear, float clipFar) override;
    };

    class DelegateLH_NO final : public Delegate {
    public:
        M4 frustum(float left, float right, float bottom, float top, float clipNear, float clipFar) override;
        M4 lookAt(const V3& position, const V3& target, const V3& up) override;
        M4 ortho(float left, float right, float bottom, float top, float clipNear, float clipFar) override;
        M4 perspective(float fov, float aspect, float clipNear, float clipFar) override;
    };

    class DelegateRH_NO final : public Delegate {
    public:
        M4 frustum(float left, float right, float bottom, float top, float clipNear, float clipFar) override;
        M4 lookAt(const V3& position, const V3& target, const V3& up) override;
        M4 ortho(float left, float right, float bottom, float top, float clipNear, float clipFar) override;
        M4 perspective(float fov, float aspect, float clipNear, float clipFar) override;
    };

public:
//  [[script::bindings::auto]]
    Camera();
    Camera(Ark::RendererCoordinateSystem cs, sp<Delegate> delegate);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Camera);

//  [[script::bindings::auto]]
    void ortho(const V2& leftTop, const V2& rightBottom, const V2& clip);
//  [[script::bindings::auto]]
    void ortho(sp<Vec2> leftTop, sp<Vec2> rightBottom, sp<Vec2> clip);
    void ortho(float left, float right, float bottom, float top, float clipNear, float clipFar);

//  [[script::bindings::auto]]
    void frustum(float left, float right, float bottom, float top, float clipNear, float clipFar);
//  [[script::bindings::auto]]
    void perspective(float fov, float aspect, float clipNear, float clipFar);

//  [[script::bindings::auto]]
    void lookAt(const V3& position, const V3& target, const V3& up);
//  [[script::bindings::auto]]
    void lookAt(sp<Vec3> position, sp<Vec3> target, sp<Vec3> up);

//  [[script::bindings::auto]]
    V3 toWorldPosition(float screenX, float screenY, float z) const;
//  [[script::bindings::auto]]
    sp<Vec3> toViewportPosition(const sp<Vec3>& position) const;

//  [[script::bindings::property]]
    sp<Vec3> position() const;
//  [[script::bindings::property]]
    sp<Vec3> target() const;

//  [[script::bindings::property]]
    sp<Mat4> view() const;
//  [[script::bindings::property]]
    void setView(sp<Mat4> view);
//  [[script::bindings::property]]
    sp<Mat4> projection() const;
//  [[script::bindings::property]]
    void setProjection(sp<Mat4> projection);
//  [[script::bindings::property]]
    sp<Mat4> vp() const;

    void assign(const Camera& other);

    static sp<Camera> getDefaultCamera();

private:
    Ark::RendererCoordinateSystem _coordinate_system;
    sp<Delegate> _delegate;

    sp<Mat4Wrapper> _view;
    sp<Mat4Wrapper> _projection;
    sp<Mat4Wrapper> _vp;

    sp<VariableWrapper<V3>> _position;
    sp<VariableWrapper<V3>> _target;
};

}
