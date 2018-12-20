#ifndef ARK_GRAPHICS_BASE_CAMERA_H_
#define ARK_GRAPHICS_BASE_CAMERA_H_

#include "core/forwarding.h"
#include "core/inf/variable.h"
#include "core/epi/changed.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/matrix.h"
#include "graphics/base/v3.h"

namespace ark {

class ARK_API Camera {
public:
//  [[script::bindings::auto]]
    Camera();

    struct Snapshot {
        Snapshot(const Matrix& vp);
        DEFAULT_COPY_AND_ASSIGN(Snapshot);

        Matrix _vp;
    };

//  [[script::bindings::auto]]
    void ortho(float left, float right, float top, float bottom, float near, float far);
//  [[script::bindings::auto]]
    void perspective(float fov, float aspect, float near, float far);

//  [[script::bindings::auto]]
    void lookAt(const V3& position, const V3& target, const V3& up);
//  [[script::bindings::auto]]
    void lookAt(const sp<Vec3>& position, const sp<Vec3>& target, const sp<Vec3>& up);

    Matrix view() const;
    Matrix projection() const;

    Snapshot snapshot() const;

    static const sp<Camera>& getMainCamera();

private:
    class Holder : public Variable<Matrix> {
    public:
        Holder(const sp<Variable<Matrix>>& delegate, const sp<Boolean>& dirty);

        virtual Matrix val() override;

    private:
        sp<Variable<Matrix>> _delegate;
        sp<Changed> _dirty;

        Matrix _value;

        friend class Camera;
    };

private:
    void updateViewProjection();

private:
    sp<Holder> _view;
    sp<Holder> _projection;

    sp<Holder> _vp;

    sp<Vec3> _position;
    sp<Vec3> _target;
    sp<Vec3> _up;

    sp<Changed> _dirty;
};

}

#endif
