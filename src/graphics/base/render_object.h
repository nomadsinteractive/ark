#ifndef ARK_GRAPHICS_BASE_RENDER_OBJECT_H_
#define ARK_GRAPHICS_BASE_RENDER_OBJECT_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/holder.h"
#include "core/types/box.h"
#include "core/types/shared_ptr.h"
#include "core/types/safe_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/block.h"
#include "graphics/base/transform.h"

#include "renderer/base/varyings.h"

namespace ark {

//[[script::bindings::holder]]
class ARK_API RenderObject : public Block, public Holder {
public:
    struct Snapshot {
        Snapshot(int32_t type, const V3& position, const V3& size, const Transform::Snapshot& transform, const Varyings::Snapshot& varyings);
        Snapshot(const Snapshot& other) = default;

        int32_t _type;
        V3 _position;
        V3 _size;
        Transform::Snapshot _transform;
        Varyings::Snapshot _varyings;
    };

public:
//  [[script::bindings::auto]]
    RenderObject(int32_t type, const sp<Vec3>& position = nullptr, const sp<Size>& size = nullptr, const sp<Transform>& transform = nullptr, const sp<Varyings>& varyings = nullptr);
//  [[script::bindings::auto]]
    RenderObject(const sp<Integer>& type, const sp<Vec3>& position = nullptr, const sp<Size>& size = nullptr, const sp<Transform>& transform = nullptr, const sp<Varyings>& varyings = nullptr);
    RenderObject(const sp<Integer>& type, const sp<Vec3>& position, const sp<Size>& size, const sp<Transform>& transform, const sp<Varyings>& varyings, const sp<Disposed>& disposed);

//  [[script::bindings::property]]
    virtual const SafePtr<Size>& size() override;
    virtual void traverse(const Visitor& visitor) override;

//  [[script::bindings::property]]
    const sp<Integer> type() const;

//  [[script::bindings::property]]
    const SafePtr<Transform>& transform() const;

//  [[script::bindings::property]]
    float width() const;
//  [[script::bindings::property]]
    float height() const;

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
    const SafePtr<Vec3>& position() const;
//  [[script::bindings::property]]
    void setPosition(const sp<Vec3>& position);
//  [[script::bindings::property]]
    void setSize(const sp<Size>& size);
//  [[script::bindings::property]]
    void setTransform(const sp<Transform>& transform);
//  [[script::bindings::property]]
    void setVaryings(const sp<Varyings>& varyings);

//  [[script::bindings::property]]
    const Box& tag() const;
//  [[script::bindings::property]]
    void setTag(const Box& tag);

//  [[script::bindings::property]]
    const sp<Disposed>& disposed() const;
//  [[script::bindings::property]]
    void setDisposed(const sp<Boolean>& disposed);

//  [[script::bindings::property]]
    const sp<Visibility>& visible() const;
//  [[script::bindings::property]]
    void setVisible(const sp<Boolean>& visible);

//  [[script::bindings::auto]]
    void dispose();

//  [[script::bindings::auto]]
    void show();
//  [[script::bindings::auto]]
    void hide();

    bool isDisposed() const;
    bool isVisible() const;

    Snapshot snapshot(MemoryPool& memoryPool) const;

//  [[plugin::builder]]
    class BUILDER : public Builder<RenderObject> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<RenderObject> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Integer>> _type;
        SafePtr<Builder<Vec3>> _position;
        SafePtr<Builder<Size>> _size;
        SafePtr<Builder<Transform>> _transform;
        SafePtr<Builder<Varyings>> _varyings;
        SafePtr<Builder<Disposed>> _disposed;
    };

private:
    sp<IntegerWrapper> _type;

    SafePtr<Vec3> _position;
    SafePtr<Size> _size;
    SafePtr<Transform> _transform;
    SafePtr<Varyings> _varyings;

    SafePtr<Disposed> _disposed;
    SafePtr<Visibility> _visible;

    Box _tag;

};

}
#endif
