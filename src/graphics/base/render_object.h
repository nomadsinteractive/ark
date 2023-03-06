#pragma once

#include "core/base/api.h"
#include "core/base/timestamp.h"
#include "core/inf/builder.h"
#include "core/inf/holder.h"
#include "core/epi/visibility.h"
#include "core/types/box.h"
#include "core/types/shared_ptr.h"
#include "core/types/safe_ptr.h"
#include "core/types/safe_var.h"

#include "graphics/forwarding.h"
#include "graphics/base/size.h"
#include "graphics/inf/renderable.h"

namespace ark {

//[[script::bindings::holder]]
class ARK_API RenderObject : public Holder, public Renderable {
public:
    RenderObject(int32_t type, sp<Vec3> position = nullptr, sp<Size> size = nullptr, sp<Transform> transform = nullptr, sp<Varyings> varyings = nullptr, sp<Boolean> visible = nullptr, sp<Boolean> disposed = nullptr);
//  [[script::bindings::auto]]
    RenderObject(sp<Integer> type, sp<Vec3> position = nullptr, sp<Size> size = nullptr, sp<Transform> transform = nullptr, sp<Varyings> varyings = nullptr, sp<Boolean> visible = nullptr, sp<Boolean> disposed = nullptr);

    virtual void traverse(const Visitor& visitor) override;

//  [[script::bindings::property]]
    sp<Integer> type() const;

//  [[script::bindings::property]]
    float width();
//  [[script::bindings::property]]
    float height();

//  [[script::bindings::property]]
    void setType(int32_t type);
//  [[script::bindings::property]]
    void setType(const sp<Integer>& type);

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
    void setPosition(const sp<Vec3>& position);
//  [[script::bindings::property]]
    const sp<Size>& size();
//  [[script::bindings::property]]
    void setSize(const sp<Size>& size);
//  [[script::bindings::property]]
    const SafePtr<Transform>& transform() const;
//  [[script::bindings::property]]
    void setTransform(const sp<Transform>& transform);
//  [[script::bindings::property]]
    const sp<Varyings>& varyings();
//  [[script::bindings::property]]
    void setVaryings(sp<Varyings> varyings);

//  [[script::bindings::property]]
    const Box& tag() const;
//  [[script::bindings::property]]
    void setTag(const Box& tag);

//  [[script::bindings::property]]
    sp<Boolean> disposed();
//  [[script::bindings::property]]
    void setDisposed(sp<Boolean> disposed);

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

    bool isDisposed() const;
    bool isVisible() const;

    virtual StateBits updateState(const RenderRequest& renderRequest) override;
    virtual Renderable::Snapshot snapshot(const PipelineInput& pipelineInput, const RenderRequest& renderRequest, const V3& postTranslate, StateBits state) override;

//  [[plugin::builder]]
    class BUILDER : public Builder<RenderObject> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<RenderObject> build(const Scope& args) override;

    private:
        SafePtr<Builder<Integer>> _type;
        SafePtr<Builder<Vec3>> _position;
        SafePtr<Builder<Size>> _size;
        SafePtr<Builder<Transform>> _transform;
        SafePtr<Builder<Varyings>> _varyings;
        SafePtr<Builder<Boolean>> _disposed;
    };

private:
    sp<IntegerWrapper> _type;

    SafeVar<Vec3> _position;
    SafeVar<Size> _size;
    SafePtr<Transform> _transform;
    sp<Varyings> _varyings;

    SafeVar<Boolean> _visible;
    SafeVar<Boolean> _disposed;

    Box _tag;

    Timestamp _timestamp;

    friend class RenderObjectWithLayer;
};

}
