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

#include "graphics/forwarding.h"
#include "graphics/inf/layout.h"
#include "graphics/inf/renderable.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API View final : public Wirable {
public:
    View(sp<LayoutParam> layoutParam, sp<RenderObjectWithLayer> background = nullptr, sp<Boolean> visible = nullptr, sp<Boolean> discarded = nullptr);
    ~View() override;

    TypeId onWire(WiringContext& context) override;

    void addRenderObjectWithLayer(sp<RenderObjectWithLayer> ro, bool isBackground);

    bool updateLayout(uint64_t timestamp) const;
    void updateTextLayout(uint64_t timestamp);

    const sp<Layout::Node>& layoutNode() const;

//  [[script::bindings::property]]
    const SafeVar<Boolean>& visible() const;
//  [[script::bindings::property]]
    void setVisbile(sp<Boolean> visible);

//  [[script::bindings::property]]
    const SafeVar<Boolean>& discarded() const;
//  [[script::bindings::property]]
    void setDiscarded(sp<Boolean> discarded);

//  [[script::bindings::property]]
    const sp<LayoutParam>& layoutParam() const;
//  [[script::bindings::property]]
    void setLayoutParam(sp<LayoutParam> layoutParam);

//  [[script::bindings::auto]]
    void addView(sp<View> view, sp<Boolean> discarded = nullptr);

    const sp<ViewHierarchy>& hierarchy() const;

    void setParent(const View& view);

//  [[plugin::builder]]
    class BUILDER : public Builder<View> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<View> build(const Scope& args) override;

    private:
        BeanFactory _factory;
        document _manifest;

        SafePtr<Builder<Boolean>> _discarded;
        SafePtr<Builder<Boolean>> _visible;
        SafePtr<Builder<RenderObjectWithLayer>> _background;
        sp<Builder<LayoutParam>> _layout_param;
    };

    struct Stub : Updatable {
        Stub(sp<LayoutParam> layoutParam, sp<Boolean> visible, sp<Boolean> discarded);

        bool update(uint64_t timestamp) override;

        void updateLayout(uint64_t timestamp);

        void dispose();

        bool isVisible() const;
        bool isDiscarded() const;

        V3 getTopViewOffsetPosition(bool includePaddings) const;
        sp<Layout::Node> getTopViewLayoutNode() const;

        const sp<ViewHierarchy>& viewHierarchy() const;
        ViewHierarchy& ensureViewHierarchy();

        sp<ViewHierarchy> _hierarchy;
        sp<Layout::Node> _layout_node;

        SafeVar<Boolean> _visible;
        SafeVar<Boolean> _discarded;

        WeakPtr<Stub> _parent_stub;
        bool _top_view;
    };

    class UpdatableIsolatedLayout : public Updatable {
    public:
        UpdatableIsolatedLayout(sp<Stub> stub);

        bool update(uint64_t timestamp) override;

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

        bool update(uint64_t timestamp) override;
        bool val() override;

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

    sp<EventListener> _on_move;
    sp<IsDiscarded> _is_discarded;
    sp<Updatable> _updatable_view;
    sp<Updatable> _updatable_layout;

    sp<Size> _size;

    friend class Arena;
    friend class ViewHierarchy;
};

}
