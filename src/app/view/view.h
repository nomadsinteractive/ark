#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/inf/updatable.h"
#include "core/inf/wirable.h"
#include "core/types/safe_var.h"

#include "graphics/forwarding.h"
#include "graphics/components/text.h"
#include "graphics/inf/layout.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API View final : public Wirable, public Wirable::Niche {
public:
//  [[script::bindings::auto]]
    View(sp<LayoutParam> layoutParam, String name = "", sp<Vec3> position = nullptr, sp<Boolean> discarded = nullptr);
    ~View() override;

    TypeId onPoll(WiringContext& context) override;
    void onWire(const WiringContext& context, const Box& self) override;

    void onPoll(WiringContext& context, const document& component) override;

    bool update(uint64_t timestamp) const;

    const sp<Layout::Node>& layoutNode() const;

//  [[script::bindings::property]]
    const String& name() const;

//  [[script::bindings::property]]
    const SafeVar<Boolean>& discarded() const;
//  [[script::bindings::property]]
    void setDiscarded(sp<Boolean> discarded);

//  [[script::bindings::property]]
    const sp<LayoutParam>& layoutParam() const;
//  [[script::bindings::property]]
    void setLayoutParam(sp<LayoutParam> layoutParam);

//  [[script::bindings::property]]
    const sp<Vec3>& layoutPosition();
//  [[script::bindings::property]]
    const sp<Size>& layoutSize();

//  [[script::bindings::auto]]
    void addView(sp<View> view);
//  [[script::bindings::auto]]
    sp<View> findView(StringView name) const;

//  [[script::bindings::auto]]
    sp<Boundaries> makeBoundaries();

    const sp<ViewHierarchy>& hierarchy() const;

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
        builder<View> _view;
    };

    struct Stub;

private:
    sp<Updatable>& ensureUpdatableLayout();

    void markAsTopView();

protected:
    sp<Stub> _stub;
    sp<Updatable> _updatable_layout;

    sp<Vec3> _layout_position;
    sp<Size> _layout_size;

    friend class Activity;
    friend class ViewHierarchy;
};

}
