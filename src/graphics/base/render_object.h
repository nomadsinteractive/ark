#ifndef ARK_GRAPHICS_BASE_RENDER_OBJECT_H_
#define ARK_GRAPHICS_BASE_RENDER_OBJECT_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/holder.h"
#include "core/epi/disposed.h"
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
//  [[script::bindings::auto]]
    RenderObject(int32_t type, const sp<Vec3>& position = nullptr, const sp<Size>& size = nullptr, const sp<Transform>& transform = nullptr, const sp<Varyings>& varyings = nullptr);
//  [[script::bindings::auto]]
    RenderObject(const sp<Integer>& type, const sp<Vec3>& position = nullptr, const sp<Size>& size = nullptr, const sp<Transform>& transform = nullptr, const sp<Varyings>& varyings = nullptr);
    RenderObject(const sp<Integer>& type, const sp<Vec3>& position, const sp<Size>& size, const sp<Transform>& transform, const sp<Varyings>& varyings, const sp<Disposed>& disposed);

    virtual void traverse(const Visitor& visitor) override;

//  [[script::bindings::property]]
    const sp<Integer> type() const;

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
    const SafePtr<Varyings>& varyings() const;
//  [[script::bindings::property]]
    void setVaryings(const sp<Varyings>& varyings);

//  [[script::bindings::property]]
    const Box& tag() const;
//  [[script::bindings::property]]
    void setTag(const Box& tag);

//  [[script::bindings::property]]
    const sp<Disposed>& disposed();
//  [[script::bindings::property]]
    void setDisposed(const sp<Boolean>& disposed);

//  [[script::bindings::property]]
    const sp<Visibility>& visible();
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

    virtual Renderable::Snapshot snapshot(const PipelineInput& pipelineInput, const RenderRequest& renderRequest) override;

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
        SafePtr<Builder<Disposed>> _disposed;
    };

private:
    sp<IntegerWrapper> _type;

    SafeVar<Vec3> _position;
    SafeVar<Size> _size;
    SafePtr<Transform> _transform;
    SafePtr<Varyings> _varyings;

    SafeVar<Disposed> _disposed;
    SafeVar<Visibility> _visible;

    Box _tag;

    friend class RendererByRenderObject;

};

}
#endif
