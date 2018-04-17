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
    Transform(const sp<Numeric>& rotation = nullptr, const sp<VV>& scale = nullptr, const sp<VV>& translation = nullptr);
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

        float rotation;
        V scale;
        V translate;
    };

    Snapshot snapshot() const;

//  [[script::bindings::property]]
    const sp<Numeric>& rotation();
//  [[script::bindings::property]]
    void setRotation(const sp<Numeric>& rotation);

//  [[script::bindings::property]]
    const sp<VV>& scale() const;
//  [[script::bindings::property]]
    void setScale(const sp<VV>& scale);

//  [[script::bindings::property]]
    const sp<VV>& translation() const;
//  [[script::bindings::property]]
    void setTranslation(const sp<VV>& translation);

//  [[plugin::builder]]
    class BUILDER : public Builder<Transform> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Transform> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _rotation;
        sp<Builder<VV>> _scale;
        sp<Builder<VV>> _translation;

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
    static const sp<VV>& identity();

private:
    SafePtr<Numeric, Numeric::Impl> _rotation;
    sp<VV> _scale;
    SafePtr<VV, VV::Impl> _translation;

};

}

#endif
