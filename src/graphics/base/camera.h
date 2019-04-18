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
        Holder(const sp<Variable<Matrix>>& delegate);

        virtual void flat(void* buf) override;
        virtual uint32_t size() override;

        Matrix matrix();

    private:
        sp<Variable<Matrix>> _delegate;

        friend class Camera;
    };

public:
//  [[script::bindings::auto]]
    Camera();

    struct Snapshot {
        Snapshot(Holder& holder);
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

    const sp<Holder>& view() const;
    const sp<Holder>& projection() const;
    const sp<Holder>& vp() const;

    const sp<Notifier>& notifier() const;

    Snapshot snapshot() const;

    static const sp<Camera>& getMainCamera();

private:
    void updateViewProjection();

private:
    sp<Holder> _view;
    sp<Holder> _projection;

    sp<Holder> _vp;

    sp<Notifier> _notifier;
};

}

#endif
