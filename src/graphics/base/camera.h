#ifndef ARK_GRAPHICS_BASE_CAMERA_H_
#define ARK_GRAPHICS_BASE_CAMERA_H_

#include "core/ark.h"
#include "core/forwarding.h"
#include "core/inf/input.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/viewport.h"
#include "graphics/base/v3.h"

namespace ark {

class ARK_API Camera {
public:
    class Holder : public Input, public Mat4 {
    public:
        Holder(sp<Mat4> value);

        virtual void flat(void* buf) override;
        virtual uint32_t size() override;
        virtual bool update(uint64_t timestamp) override;

        virtual M4 val() override;

        const sp<Mat4>& matrix() const;
        void setMatrix(sp<Mat4> matrix);

    private:
        sp<Mat4> _matrix;
        Timestamp _timestamp;
    };

    class Delegate {
    public:
        virtual ~Delegate() = default;

        virtual M4 frustum(float left, float right, float bottom, float top, float clipNear, float clipFar) = 0;
        virtual M4 lookAt(const V3& position, const V3& target, const V3& up) = 0;
        virtual M4 ortho(float left, float right, float bottom, float top, float clipNear, float clipFar) = 0;
        virtual M4 perspective(float fov, float aspect, float clipNear, float clipFar) = 0;
    };

    class DelegateLH_ZO : public Delegate {
    public:
        virtual M4 frustum(float left, float right, float bottom, float top, float clipNear, float clipFar) override;
        virtual M4 lookAt(const V3& position, const V3& target, const V3& up) override;
        virtual M4 ortho(float left, float right, float bottom, float top, float clipNear, float clipFar) override;
        virtual M4 perspective(float fov, float aspect, float clipNear, float clipFar) override;
    };

    class DelegateRH_ZO : public Delegate {
    public:
        virtual M4 frustum(float left, float right, float bottom, float top, float clipNear, float clipFar) override;
        virtual M4 lookAt(const V3& position, const V3& target, const V3& up) override;
        virtual M4 ortho(float left, float right, float bottom, float top, float clipNear, float clipFar) override;
        virtual M4 perspective(float fov, float aspect, float clipNear, float clipFar) override;
    };

    class DelegateLH_NO : public Delegate {
    public:
        virtual M4 frustum(float left, float right, float bottom, float top, float clipNear, float clipFar) override;
        virtual M4 lookAt(const V3& position, const V3& target, const V3& up) override;
        virtual M4 ortho(float left, float right, float bottom, float top, float clipNear, float clipFar) override;
        virtual M4 perspective(float fov, float aspect, float clipNear, float clipFar) override;
    };

    class DelegateRH_NO : public Delegate {
    public:
        virtual M4 frustum(float left, float right, float bottom, float top, float clipNear, float clipFar) override;
        virtual M4 lookAt(const V3& position, const V3& target, const V3& up) override;
        virtual M4 ortho(float left, float right, float bottom, float top, float clipNear, float clipFar) override;
        virtual M4 perspective(float fov, float aspect, float clipNear, float clipFar) override;
    };

public:
//  [[script::bindings::auto]]
    Camera();
    Camera(Ark::RendererCoordinateSystem cs, sp<Delegate> delegate);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Camera);

//  [[script::bindings::auto]]
    void ortho(float left, float right, float bottom, float top, float clipNear, float clipFar);
    void ortho(float left, float right, float bottom, float top, float clipNear, float clipFar, Ark::RendererCoordinateSystem coordinateSystem);
//  [[script::bindings::auto]]
    void frustum(float left, float right, float bottom, float top, float clipNear, float clipFar);
//  [[script::bindings::auto]]
    void perspective(float fov, float aspect, float clipNear, float clipFar);

//  [[script::bindings::auto]]
    void lookAt(const V3& position, const V3& target, const V3& up);
//  [[script::bindings::auto]]
    void lookAt(const sp<Vec3>& position, const sp<Vec3>& target, const sp<Vec3>& up);

//  [[script::bindings::auto]]
    V3 toWorldPosition(float screenX, float screenY, float z) const;
//  [[script::bindings::auto]]
    sp<Vec3> toScreenPosition(const sp<Vec3>& position) const;

//  [[script::bindings::property]]
    sp<Vec3> position() const;
//  [[script::bindings::property]]
    sp<Vec3> target() const;

//  [[script::bindings::property]]
    sp<Mat4> matrixView() const;
//  [[script::bindings::property]]
    sp<Mat4> matrixProjection() const;
//  [[script::bindings::property]]
    sp<Mat4> matrixViewProjection() const;

    const sp<Holder>& view() const;
    const sp<Holder>& projection() const;
    const sp<Holder>& vp() const;

    static const sp<Camera>& getDefaultCamera();

private:
    void updateViewProjection();

private:
    Ark::RendererCoordinateSystem _coordinate_system;
    sp<Delegate> _delegate;

    sp<Holder> _view;
    sp<Holder> _projection;

    sp<Holder> _vp;

    sp<VariableWrapper<V3>> _position;
    sp<VariableWrapper<V3>> _target;
};

}

#endif
