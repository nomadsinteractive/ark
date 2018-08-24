#ifndef ARK_GRAPHICS_BASE_TRANSFORM_H_
#define ARK_GRAPHICS_BASE_TRANSFORM_H_

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/inf/dictionary.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"
#include "core/types/safe_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

namespace ark {

class ARK_API Transform {
public:
//  [[script::bindings::auto]]
    Transform(const sp<Rotation>& rotate = nullptr, const sp<Vec>& scale = nullptr, const sp<Vec>& translation = nullptr);
    Transform(const Transform& other);

    class ARK_API Snapshot {
    public:
        Snapshot();
        Snapshot(const Snapshot& other) = default;

        Matrix toMatrix() const;
        void toMatrix(Matrix& matrix) const;

        bool isFrontfaceCCW() const;

        bool operator ==(const Snapshot& other) const;
        bool operator !=(const Snapshot& other) const;

        void map(float x, float y, float tx, float ty, float& mx, float& my) const;
        V3 mapXYZ(const V3& p) const;

        float rotate_value;
        V3 rotate_direction;
        V3 scale;
        V3 translate;
    };

    Snapshot snapshot() const;

//  [[script::bindings::property]]
    const sp<Rotation>& rotate();
//  [[script::bindings::property]]
    void setRotate(const sp<Rotation>& rotate);

//  [[script::bindings::property]]
    const sp<Vec>& scale() const;
//  [[script::bindings::property]]
    void setScale(const sp<Vec>& scale);

//  [[script::bindings::property]]
    const sp<Vec>& translation() const;
//  [[script::bindings::property]]
    void setTranslation(const sp<Vec>& translation);

//  [[plugin::builder]]
    class BUILDER : public Builder<Transform> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Transform> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Rotation>> _rotate;
        sp<Builder<Vec>> _scale;
        sp<Builder<Vec>> _translation;

    };

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Transform> {
    public:
        DICTIONARY(BeanFactory& parent, const String& value);

        virtual sp<Transform> build(const sp<Scope>& args) override;

    private:
        BUILDER _impl;

    };

private:
    SafePtr<Rotation> _rotate;
    sp<Vec> _scale;
    SafePtr<Vec, Vec::Impl> _translation;

};

}

#endif
