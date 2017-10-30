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
#include "graphics/base/v2.h"

namespace ark {

class ARK_API Transform {
public:
//  [[script::bindings::auto]]
    Transform(const sp<VV2>& pivot = nullptr, const sp<Numeric>& rotation = nullptr, const sp<VV2>& scale = nullptr, const sp<VV2>& translation = nullptr);
    Transform(const Transform& other);
//    Transform();

    class ARK_API Snapshot {
    public:
        Snapshot(float px, float py);
        Snapshot(const Snapshot& other) = default;

        Matrix toMatrix() const;
        void toMatrix(Matrix& matrix) const;

        bool isFrontfaceCCW() const;

        bool operator ==(const Snapshot& other) const;
        bool operator !=(const Snapshot& other) const;

        void map(float x, float y, float tx, float ty, float& mx, float& my) const;

        float pivot_x, pivot_y;
        float rotation;
        V2 scale;
        V2 translate;
    };

    Snapshot snapshot(float px = 0, float py = 0) const;

//  [[script::bindings::property]]
    const sp<VV2>& pivot() const;
//  [[script::bindings::property]]
    void setPivot(const sp<VV2>& pivot);

//  [[script::bindings::property]]
    const sp<Numeric>& rotation();
//  [[script::bindings::property]]
    void setRotation(const sp<Numeric>& rotation);

//  [[script::bindings::property]]
    const sp<VV2>& scale() const;
//  [[script::bindings::property]]
    void setScale(const sp<VV2>& scale);

//  [[script::bindings::property]]
    const sp<VV2>& translation() const;
//  [[script::bindings::property]]
    void setTranslation(const sp<VV2>& translation);

//  [[plugin::builder]]
    class BUILDER : public Builder<Transform> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Transform> build(const sp<Scope>& args) override;

    private:
        sp<Builder<VV2>> _pivot;
        sp<Builder<Numeric>> _rotation;
        sp<Builder<VV2>> _scale;
        sp<Builder<VV2>> _translation;

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
    sp<VV2> _pivot;
    SafePtr<Numeric, Numeric::Impl> _rotation;
    sp<VV2> _scale;
    SafePtr<VV2, VV2::Impl> _translation;

};

}

#endif
