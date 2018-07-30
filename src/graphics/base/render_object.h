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
        Snapshot(uint32_t type, const V& position, const V& size, const Transform::Snapshot& transform, const Varyings::Snapshot& varyings);
        Snapshot(const Snapshot& other) = default;

        uint32_t _type;
        V _position;
        V _size;
        Transform::Snapshot _transform;
        Varyings::Snapshot _varyings;
    };

public:
//  [[script::bindings::auto]]
    RenderObject(int32_t type, const sp<Vec>& position = nullptr, const sp<Size>& size = nullptr, const sp<Transform>& transform = nullptr, const sp<Varyings>& varyings = nullptr);
//  [[script::bindings::auto]]
    RenderObject(const sp<Integer>& type, const sp<Vec>& position = nullptr, const sp<Size>& size = nullptr, const sp<Transform>& transform = nullptr, const sp<Varyings>& varyings = nullptr);

//  [[script::bindings::meta(absorb())]]
//  [[script::bindings::meta(expire())]]
//  [[script::bindings::meta(isExpired())]]

//  [[script::bindings::property]]
    const sp<Integer> type() const;

//  [[script::bindings::property]]
    virtual const sp<Size>& size() override;

//  [[script::bindings::property]]
    const sp<Transform>& transform() const;
//  [[script::bindings::property]]
    const sp<Varyings>& varyings() const;

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
    float y() const;
//  [[script::bindings::property]]
    V2 xy() const;
//  [[script::bindings::property]]
    const sp<Vec>& position() const;
//  [[script::bindings::property]]
    void setPosition(const sp<Vec>& position);
//  [[script::bindings::property]]
    void setSize(const sp<Size>& size);
//  [[script::bindings::property]]
    void setTransform(const sp<Transform>& transform);
//  [[script::bindings::property]]
    void setVaryings(const sp<Varyings>& filter);

//  [[script::bindings::property]]
    const Box& tag() const;
//  [[script::bindings::property]]
    void setTag(const Box& tag);

    bool isExpired() const;

    Snapshot snapshot(MemoryPool& memoryPool) const;

//  [[plugin::builder]]
    class BUILDER : public Builder<RenderObject> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<RenderObject> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Integer>> _type;
        sp<Builder<Vec>> _position;
        sp<Builder<Size>> _size;
        sp<Builder<Transform>> _transform;
        sp<Builder<Varyings>> _varyings;
    };

//  [[plugin::style("expired")]]
    class EXPIRED_STYLE : public Builder<RenderObject> {
    public:
        EXPIRED_STYLE(BeanFactory& factory, const sp<Builder<RenderObject>>& delegate, const String& value);

        virtual sp<RenderObject> build(const sp<Scope>& args) override;

    private:
        sp<Builder<RenderObject>> _delegate;
        sp<Builder<Expired>> _expired;

    };

private:
    sp<IntegerWrapper> _type;

    SafePtr<Vec, VecImpl> _position;
    SafePtr<Size> _size;
    SafePtr<Transform> _transform;
    sp<Varyings> _varyings;

    sp<Boolean> _type_expired;

    Box _tag;
};

}
#endif
