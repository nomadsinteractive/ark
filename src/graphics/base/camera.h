#ifndef ARK_GRAPHICS_BASE_CAMERA_H_
#define ARK_GRAPHICS_BASE_CAMERA_H_

#include "core/forwarding.h"
#include "core/inf/flatable.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/matrix.h"
#include "graphics/base/viewport.h"
#include "graphics/base/v3.h"

namespace ark {

class ARK_API Camera {
public:
    class Holder : public Flatable {
    public:
        Holder(const sp<Variable<Matrix>>& value);

        virtual void flat(void* buf) override;
        virtual uint32_t size() override;

    private:
        sp<Variable<Matrix>> _value;

        friend class Camera;
    };

    class Delegate {
    public:
        virtual ~Delegate() = default;

        virtual Matrix frustum(float left, float right, float bottom, float top, float near, float far) = 0;
        virtual Matrix lookAt(const V3& position, const V3& target, const V3& up) = 0;
        virtual Matrix ortho(float left, float right, float bottom, float top, float near, float far) = 0;
        virtual Matrix perspective(float fov, float aspect, float near, float far) = 0;
    };

    class DelegateLH_ZO : public Delegate {
    public:
        virtual Matrix frustum(float left, float right, float bottom, float top, float near, float far) override;
        virtual Matrix lookAt(const V3& position, const V3& target, const V3& up) override;
        virtual Matrix ortho(float left, float right, float bottom, float top, float near, float far) override;
        virtual Matrix perspective(float fov, float aspect, float near, float far) override;
    };

    class DelegateRH_NO : public Delegate {
    public:
        virtual Matrix frustum(float left, float right, float bottom, float top, float near, float far) override;
        virtual Matrix lookAt(const V3& position, const V3& target, const V3& up) override;
        virtual Matrix ortho(float left, float right, float bottom, float top, float near, float far) override;
        virtual Matrix perspective(float fov, float aspect, float near, float far) override;
    };

public:
//  [[script::bindings::auto]]
    Camera();

//  [[script::bindings::auto]]
    void ortho(float left, float right, float bottom, float top, float near, float far, float upDirection = 1.0f);
//  [[script::bindings::auto]]
    void frustum(float left, float right, float bottom, float top, float near, float far);
//  [[script::bindings::auto]]
    void perspective(float fov, float aspect, float near, float far);

//  [[script::bindings::auto]]
    void lookAt(const V3& position, const V3& target, const V3& up);
//  [[script::bindings::auto]]
    void lookAt(const sp<Vec3>& position, const sp<Vec3>& target, const sp<Vec3>& up);

    const sp<Holder>& view() const;
    const sp<Holder>& projection() const;
    const sp<Holder>& vp() const;

    const sp<Notifier>& notifier() const;

//    Snapshot snapshot() const;

    static const sp<Camera>& getDefaultCamera();

private:
    void updateViewProjection();

private:
    sp<Delegate> _delegate;

    sp<Holder> _view;
    sp<Holder> _projection;

    sp<Holder> _vp;

    sp<Notifier> _notifier;
};

}

#endif
