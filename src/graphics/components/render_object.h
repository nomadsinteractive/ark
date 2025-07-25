#pragma once

#include "core/base/api.h"
#include "core/base/timestamp.h"
#include "core/inf/builder.h"
#include "core/inf/wirable.h"
#include "core/types/box.h"
#include "core/types/shared_ptr.h"
#include "core/types/safe_var.h"

#include "graphics/forwarding.h"
#include "graphics/components/size.h"
#include "graphics/inf/renderable.h"

namespace ark {

class ARK_API RenderObject final : public Renderable, public Wirable {
public:
//  [[script::bindings::auto]]
    RenderObject(sp<Integer> type, sp<Vec3> position = nullptr, sp<Vec3> size = nullptr, sp<Transform> transform = nullptr, sp<Varyings> varyings = nullptr, sp<Boolean> visible = nullptr, sp<Boolean> discarded = nullptr);
//  [[script::bindings::auto]]
    RenderObject(const NamedHash& type, sp<Vec3> position = nullptr, sp<Vec3> size = nullptr, sp<Transform> transform = nullptr, sp<Varyings> varyings = nullptr, sp<Boolean> visible = nullptr, sp<Boolean> discarded = nullptr);

//  [[script::bindings::property]]
    sp<Integer> type() const;

//  [[script::bindings::property]]
    void setType(const NamedHash& type);
//  [[script::bindings::property]]
    void setType(sp<Integer> type);

//  [[script::bindings::property]]
    sp<Vec3> position() const;
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
    Box tag() const;
//  [[script::bindings::property]]
    void setTag(Box tag);

//  [[script::bindings::property]]
    sp<Boolean> discarded() const;
//  [[script::bindings::property]]
    void setDiscarded(sp<Boolean> discarded);

//  [[script::bindings::property]]
    sp<Boolean> visible() const;
//  [[script::bindings::property]]
    void setVisible(bool visible);
//  [[script::bindings::property]]
    void setVisible(sp<Boolean> visible);

//  [[script::bindings::auto]]
    void discard();

//  [[script::bindings::auto]]
    void show();
//  [[script::bindings::auto]]
    void hide();

    State updateState(const RenderRequest& renderRequest) override;
    Snapshot snapshot(const LayerContextSnapshot& snapshotContext, const RenderRequest& renderRequest, State state) override;

    void onWire(const WiringContext& context, const Box& self) override;

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
        SafeBuilder<Boolean> _visible;
        SafeBuilder<Boolean> _discarded;
    };

//  [[plugin::builder("with-render-object")]]
    class BUILDER_WIRABLE final : public Builder<Wirable> {
    public:
        BUILDER_WIRABLE(BeanFactory& factory, const document& manifest);

        sp<Wirable> build(const Scope& args) override;

    private:
        builder<RenderObject> _render_object;
    };

private:
    sp<IntegerWrapper> _type;

    SafeVar<Vec3> _position;
    SafeVar<Vec3> _size;
    sp<Transform> _transform;
    sp<Varyings> _varyings;

    SafeVar<Boolean> _visible;
    SafeVar<Boolean> _discarded;

    sp<Tags> _tags;

    Timestamp _timestamp;
};

}
