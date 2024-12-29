#pragma once

#include "core/base/api.h"
#include "core/base/timestamp.h"
#include "core/inf/builder.h"
#include "core/types/box.h"
#include "core/types/shared_ptr.h"
#include "core/types/safe_var.h"

#include "graphics/forwarding.h"
#include "graphics/base/size.h"
#include "graphics/inf/renderable.h"

namespace ark {

class ARK_API RenderObject final : public Renderable {
public:
//  [[script::bindings::auto]]
    RenderObject(sp<Integer> type, sp<Vec3> position = nullptr, sp<Vec3> size = nullptr, sp<Transform> transform = nullptr, sp<Varyings> varyings = nullptr, sp<Boolean> visible = nullptr, sp<Boolean> discarded = nullptr);
//  [[script::bindings::auto]]
    RenderObject(const NamedHash& type, sp<Vec3> position = nullptr, sp<Vec3> size = nullptr, sp<Transform> transform = nullptr, sp<Varyings> varyings = nullptr, sp<Boolean> visible = nullptr, sp<Boolean> discarded = nullptr);

//  [[script::bindings::property]]
    sp<Integer> type() const;

//  [[script::bindings::property]]
    float width();
//  [[script::bindings::property]]
    float height();

//  [[script::bindings::property]]
    void setType(const NamedHash& type);
//  [[script::bindings::property]]
    void setType(sp<Integer> type);

//  [[script::bindings::property]]
    float x() const;
//  [[script::bindings::property]]
    void setX(float x);
//  [[script::bindings::property]]
    void setX(const sp<Numeric>& x);
//  [[script::bindings::property]]
    float y() const;
//  [[script::bindings::property]]
    void setY(float y);
//  [[script::bindings::property]]
    void setY(const sp<Numeric>& y);
//  [[script::bindings::property]]
    float z() const;
//  [[script::bindings::property]]
    void setZ(float z);
//  [[script::bindings::property]]
    void setZ(const sp<Numeric>& z);

//  [[script::bindings::property]]
    V2 xy() const;
//  [[script::bindings::property]]
    V3 xyz() const;

//  [[script::bindings::property]]
    const sp<Vec3>& position();
//  [[script::bindings::property]]
    void setPosition(sp<Vec3> position);
//  [[script::bindings::property]]
    const SafeVar<Vec3>& size();
//  [[script::bindings::property]]
    void setSize(sp<Vec3> size);
//  [[script::bindings::property]]
    const sp<Transform>& transform() const;
//  [[script::bindings::property]]
    void setTransform(sp<Transform> transform);
//  [[script::bindings::property]]
    const sp<Varyings>& varyings();
//  [[script::bindings::property]]
    void setVaryings(sp<Varyings> varyings);

//  [[script::bindings::property]]
    const Box& tag() const;
//  [[script::bindings::property]]
    void setTag(const Box& tag);

//  [[script::bindings::property]]
    const sp<Boolean>& discarded();
//  [[script::bindings::property]]
    void setDiscarded(sp<Boolean> discarded);

//  [[script::bindings::property]]
    sp<Boolean> visible();
//  [[script::bindings::property]]
    void setVisible(bool visible);
//  [[script::bindings::property]]
    void setVisible(sp<Boolean> visible);

//  [[script::bindings::auto]]
    void dispose();

//  [[script::bindings::auto]]
    void show();
//  [[script::bindings::auto]]
    void hide();

    bool isDiscarded() const;
    bool isVisible() const;

    StateBits updateState(const RenderRequest& renderRequest) override;
    Renderable::Snapshot snapshot(const LayerContextSnapshot& snapshotContext, const RenderRequest& renderRequest, StateBits state) override;

//  [[plugin::builder]]
    class BUILDER final : public Builder<RenderObject> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<RenderObject> build(const Scope& args) override;

    private:
        SafeBuilder<Integer> _type;
        SafeBuilder<Vec3> _position;
        SafeBuilder<Size> _size;
        SafeBuilder<Transform> _transform;
        SafeBuilder<Varyings> _varyings;
        SafeBuilder<Boolean> _discarded;
    };

//  [[plugin::builder("render_object")]]
    class BUILDER_RENDERABLE final : public Builder<Renderable> {
    public:
        BUILDER_RENDERABLE(BeanFactory& factory, const document& manifest);

        sp<Renderable> build(const Scope& args) override;

    private:
        BUILDER _builder_impl;
    };


//  [[plugin::builder("with-render-object")]]
    class BUILDER_WIRABLE final : public Builder<Wirable> {
    public:
        BUILDER_WIRABLE(BeanFactory& factory, const document& manifest);

        sp<Wirable> build(const Scope& args) override;

    private:
        BUILDER _builder_impl;
    };

private:
    sp<IntegerWrapper> _type;

    SafeVar<Vec3> _position;
    SafeVar<Vec3> _size;
    sp<Transform> _transform;
    sp<Varyings> _varyings;

    SafeVar<Boolean> _visible;
    SafeVar<Boolean> _discarded;

    Box _tag;

    Timestamp _timestamp;
};

}
