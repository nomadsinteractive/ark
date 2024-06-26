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
    View(sp<LayoutParam> layoutParam, sp<RenderObject> background = nullptr, sp<Boolean> visible = nullptr, sp<Boolean> discarded = nullptr);
    ~View() override;

    TypeId onPoll(WiringContext& context) override;
    void onWire(const WiringContext& context) override;

    bool updateLayout(uint64_t timestamp) const;

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
        SafePtr<Builder<RenderObject>> _background;
        sp<Builder<LayoutParam>> _layout_param;
    };

//  [[plugin::builder("view")]]
    class BUILDER_WIRABLE : public Builder<Wirable> {
    public:
        BUILDER_WIRABLE(BeanFactory& factory, const document& manifest);

        sp<Wirable> build(const Scope& args) override;

    private:
        BUILDER _builder_impl;
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

private:
    void markAsTopView();

protected:
    sp<Stub> _stub;
    sp<RenderObject> _background;
    sp<Boolean> _is_discarded;
    sp<Updatable> _updatable_view;
    sp<Updatable> _updatable_layout;

    friend class Arena;
    friend class ViewHierarchy;
};

}
