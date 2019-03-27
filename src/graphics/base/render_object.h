#ifndef ARK_GRAPHICS_BASE_RENDER_OBJECT_H_
#define ARK_GRAPHICS_BASE_RENDER_OBJECT_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/types/box.h"
#include "core/types/shared_ptr.h"
#include "core/types/safe_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/block.h"
#include "graphics/base/transform.h"

#include "renderer/base/varyings.h"

namespace ark {

//[[script::bindings::container]]
class ARK_API RenderObject : public Block {
public:
    struct Snapshot {
        Snapshot(int32_t type, const V& position, const V3& size, const Transform::Snapshot& transform, const Varyings::Snapshot& varyings);
        Snapshot(const Snapshot& other) = default;

        int32_t _type;
        V _position;
        V3 _size;
        Transform::Snapshot _transform;
        Varyings::Snapshot _varyings;
    };

public:
//  [[script::bindings::auto]]
    RenderObject(int32_t type, const sp<Vec>& position = nullptr, const sp<Size>& size = nullptr, const sp<Transform>& transform = nullptr, const sp<Varyings>& varyings = nullptr);
//  [[script::bindings::auto]]
    RenderObject(const sp<Integer>& type, const sp<Vec>& position = nullptr, const sp<Size>& size = nullptr, const sp<Transform>& transform = nullptr, const sp<Varyings>& varyings = nullptr);

//  [[script::bindings::meta(expire())]]
//  [[script::bindings::meta(isExpired())]]

//  [[script::bindings::property]]
    const sp<Integer> type() const;

//  [[script::bindings::property]]
    virtual const SafePtr<Size>& size() override;

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
    V2 xy() const;
//  [[script::bindings::property]]
    const SafePtr<Vec>& position() const;
//  [[script::bindings::property]]
    void setPosition(const sp<Vec>& position);
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
    const SafePtr<Disposed>& disposed() const;

//  [[script::bindings::auto]]
    void dispose();

    bool isDisposed() const;

    Snapshot snapshot(MemoryPool& memoryPool) const;

//  [[plugin::builder]]
    class BUILDER : public Builder<RenderObject> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<RenderObject> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Integer>> _type;
        SafePtr<Builder<Vec>> _position;
        SafePtr<Builder<Size>> _size;
        SafePtr<Builder<Transform>> _transform;
        SafePtr<Builder<Varyings>> _varyings;
    };

//  [[plugin::style("expired")]]
    class EXPIRED_STYLE : public Builder<RenderObject> {
    public:
        EXPIRED_STYLE(BeanFactory& factory, const sp<Builder<RenderObject>>& delegate, const String& value);

        virtual sp<RenderObject> build(const sp<Scope>& args) override;

    private:
        sp<Builder<RenderObject>> _delegate;
        sp<Builder<Disposed>> _disposable;

    };

private:
    sp<IntegerWrapper> _type;

    SafePtr<Vec> _position;
    SafePtr<Size> _size;
    SafePtr<Transform> _transform;
    SafePtr<Varyings> _varyings;

    SafePtr<Disposed> _disposed;

    Box _tag;
};

}
#endif
