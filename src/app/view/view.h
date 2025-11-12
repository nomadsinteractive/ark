#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/inf/updatable.h"
#include "core/inf/wirable.h"
#include "core/types/optional_var.h"

#include "graphics/forwarding.h"
#include "graphics/components/text.h"
#include "graphics/inf/layout.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API View final : public Wirable, public Wirable::Niche {
public:
    struct Node;
    struct Stub;

//  [[script::bindings::auto]]
    View(sp<LayoutParam> layoutParam, String name = "", sp<Vec3> position = nullptr, sp<Boolean> discarded = nullptr);
    View(sp<Stub> stub);

    ~View() override;

    void onPoll(WiringContext& context) override;
    void onWire(const WiringContext& context, const Box& self) override;

    void onPoll(WiringContext& context, const document& component) override;

    bool update(uint32_t tick) const;

    const sp<Layout::Node>& layoutNode() const;

//  [[script::bindings::property]]
    const String& name() const;

//  [[script::bindings::property]]
    const OptionalVar<Boolean>& discarded() const;
//  [[script::bindings::property]]
    void setDiscarded(sp<Boolean> discarded) const;

//  [[script::bindings::property]]
    const sp<LayoutParam>& layoutParam() const;
//  [[script::bindings::property]]
    void setLayoutParam(sp<LayoutParam> layoutParam) const;

//  [[script::bindings::property]]
    const sp<Vec3>& layoutPosition();
//  [[script::bindings::property]]
    const sp<Size>& layoutSize();

//  [[script::bindings::auto]]
    void addView(sp<View> view) const;
//  [[script::bindings::auto]]
    sp<View> findView(StringView name) const;

//  [[script::bindings::auto]]
    sp<Boundaries> makeBoundaries();

    const sp<ViewHierarchy>& hierarchy() const;

//  [[script::bindings::property]]
    sp<View> parent() const;
//  [[script::bindings::property]]
    void setParent(const View& view);

//  [[plugin::builder]]
    class BUILDER final : public Builder<View> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<View> build(const Scope& args) override;

    private:
        String _name;
        SafeBuilder<Vec3> _position;
        SafeBuilder<Boolean> _discarded;
        SafeBuilder<LayoutParam> _layout_param;
        Vector<builder<View>> _children;
    };

//  [[plugin::builder("with-view")]]
    class BUILDER_WIRABLE final : public Builder<Wirable> {
    public:
        BUILDER_WIRABLE(BeanFactory& factory, const document& manifest);

        sp<Wirable> build(const Scope& args) override;

    private:
        sp<Builder<View>> _view;
    };

private:
    sp<Updatable>& ensureUpdatableLayout();
    void markAsTopView();

private:
    sp<Stub> _stub;

    friend class Activity;
    friend class ViewHierarchy;
};

}
