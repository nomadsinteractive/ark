#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/inf/updatable.h"
#include "core/inf/wirable.h"
#include "core/types/safe_ptr.h"
#include "core/types/safe_var.h"
#include "core/types/weak_ptr.h"

#include "graphics/inf/renderable.h"
#include "graphics/forwarding.h"

#include "app/forwarding.h"
#include "app/inf/layout.h"

namespace ark {

class ARK_API View final : public Wirable {
public:
    View(const sp<LayoutParam>& layoutParam, sp<RenderObjectWithLayer> background = nullptr, sp<Text> text = nullptr, sp<Layout> layout = nullptr, sp<LayoutV3> layoutV3 = nullptr, sp<Boolean> visible = nullptr, sp<Boolean> disposed = nullptr);
    ~View() override;

    std::vector<std::pair<TypeId, Box>> onWire(const Traits& components) override;

    void addRenderObjectWithLayer(sp<RenderObjectWithLayer> ro, bool isBackground);

    bool updateLayout(uint64_t timestamp) const;
    void updateTextLayout(uint64_t timestamp);

    const sp<LayoutV3::Node>& layoutNode() const;

//  [[script::bindings::property]]
    const sp<Boolean>& visible() const;
//  [[script::bindings::property]]
    void setVisbile(sp<Boolean> visible);

//  [[script::bindings::property]]
    sp<Boolean> disposed() const;
//  [[script::bindings::property]]
    void setDisposed(sp<Boolean> disposed);

//  [[script::bindings::property]]
    const sp<LayoutParam>& layoutParam() const;
//  [[script::bindings::property]]
    void setLayoutParam(sp<LayoutParam> layoutParam);

//  [[script::bindings::auto]]
    void addView(sp<View> view, sp<Boolean> disposable = nullptr);

    void setParent(const View& view);

//  [[plugin::builder]]
    class BUILDER : public Builder<View> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<View> build(const Scope& args) override;

    private:
        BeanFactory _factory;
        document _manifest;

        SafePtr<Builder<Boolean>> _disposed;
        SafePtr<Builder<Boolean>> _visible;
        SafePtr<Builder<Layout>> _layout;
        SafePtr<Builder<LayoutV3>> _layout_v3;
        SafePtr<Builder<RenderObjectWithLayer>> _background;
        SafePtr<Builder<Text>> _text;
        sp<Builder<LayoutParam>> _layout_param;
    };

    struct Stub : public Updatable {
        Stub();
        Stub(sp<LayoutParam> layoutParam, sp<ViewHierarchy> viewHierarchy, sp<Boolean> visible, sp<Boolean> discarded);

        virtual bool update(uint64_t timestamp) override;

        void updateLayout(uint64_t timestamp);

        void dispose();

        bool isVisible() const;
        bool isDisposed() const;

        V3 getTopViewOffsetPosition(bool includePaddings) const;
        sp<LayoutV3::Node> getTopViewLayoutNode() const;

        const sp<ViewHierarchy>& viewHierarchy() const;
        ViewHierarchy& ensureViewHierarchy();

        sp<LayoutParam> _layout_param;
        sp<LayoutV3::Node> _layout_node;

        SafeVar<Boolean> _visible;
        SafeVar<Boolean> _discarded;

        WeakPtr<Stub> _parent_stub;
        bool _top_view;
    };

    class UpdatableIsolatedLayout : public Updatable {
    public:
        UpdatableIsolatedLayout(sp<Stub> stub);

        virtual bool update(uint64_t timestamp) override;

    private:
        sp<Stub> _stub;
    };

private:
    void markAsTopView();

private:
    class RenderableView : public Renderable {
    public:
        RenderableView(sp<Stub> viewStub, sp<Renderable> renderable, sp<ModelLoader> modelLoader, bool isBackground);

        StateBits updateState(const RenderRequest& renderRequest) override;
        Snapshot snapshot(const PipelineInput& pipelineInput, const RenderRequest& renderRequest, const V3& postTranslate, StateBits state) override;

    private:
        sp<Stub> _view_stub;
        sp<Renderable> _renderable;
        sp<ModelLoader> _model_loader;
        bool _is_background;
    };

    class IsDiscarded : public Boolean {
    public:
        IsDiscarded(sp<Stub> stub);

        virtual bool update(uint64_t timestamp) override;
        virtual bool val() override;

    private:
        sp<Stub> _stub;
    };

    class LayoutPosition : public Vec3 {
    public:
        LayoutPosition(sp<Stub> stub, sp<Updatable> updatable, bool isBackground, bool isCenter);

        bool update(uint64_t timestamp) override;
        V3 val() override;

    private:
        sp<Stub> _stub;
        sp<Updatable> _updatable;

        bool _is_background;
        bool _is_center;
    };

protected:
    sp<Stub> _stub;
    sp<RenderObjectWithLayer> _background;
    sp<Text> _text;

    sp<EventListener> _on_move;
    sp<IsDiscarded> _is_discarded;
    sp<Updatable> _is_stub_dirty;
    sp<Updatable> _is_layout_dirty;

    sp<Size> _size;

    friend class Arena;
    friend class ViewHierarchy;
};

}
