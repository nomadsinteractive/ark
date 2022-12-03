#include "dear-imgui/base/widget_type.h"

#include "core/ark.h"
#include "core/base/delegate.h"
#include "core/types/implements.h"

#include "graphics/inf/renderer.h"

#include "app/base/application_context.h"

#include "dear-imgui/widget/widget_with_visibility.h"

namespace ark {
namespace plugin {
namespace dear_imgui {

namespace {

class RendererWidget : public Renderer {
public:
    RendererWidget(sp<Widget> widget)
        : _widget(std::move(widget)) {
    }

    virtual void render(RenderRequest& /*renderRequest*/, const V3& /*position*/) override {
        _widget->render();
    }

private:
    sp<Widget> _widget;
};

class WidgetWrapper : public Widget, public Delegate<Widget>, public Implements<WidgetWrapper, Delegate<Widget>, Widget> {
public:
    WidgetWrapper(sp<Widget> delegate)
        : Delegate(std::move(delegate)) {
    }

    virtual void render() override {
        if(_delegate)
            _delegate->render();
    }
};

class WidgetBefore : public Widget, public Delegate<Widget>, public Implements<WidgetBefore, Delegate<Widget>, Widget> {
public:
    WidgetBefore(sp<Widget> before, sp<Widget> after)
        : Delegate(std::move(after)), _before(std::move(before)) {
    }

    virtual void render() override {
        _before->render();
        if(_delegate)
            _delegate->render();
    }

private:
    sp<Widget> _before;
};

}

sp<Widget> WidgetType::create(sp<Widget> wrapped)
{
    return sp<WidgetWrapper>::make(std::move(wrapped));
}

sp<Widget> WidgetType::makeVisible(sp<Widget> self, sp<Boolean> visibility)
{
    return sp<WidgetWithVisibility>::make(self, visibility);
}

sp<Widget> WidgetType::before(sp<Widget> self, sp<Widget> after)
{
    return sp<WidgetBefore>::make(std::move(self), std::move(after));
}

sp<Widget> WidgetType::wrapped(sp<Widget> self)
{
    sp<Delegate<Widget>> wrapper = self.as<Delegate<Widget>>();
    WARN(wrapper, "Must be a Wrapper<Widget> instance to get its wrapped attribute");
    return wrapper ? wrapper->delegate() : nullptr;
}

void WidgetType::setWrapped(const sp<Widget>& self, sp<Widget> wrapped)
{
    sp<Delegate<Widget>> wrapper = self.as<Delegate<Widget>>();
    CHECK(wrapper, "Must be a Wrapper<Widget> instance to set its wrapped attribute");
    Ark::instance().applicationContext()->deferUnref(wrapper->reset(std::move(wrapped)));
}

sp<Renderer> WidgetType::toRenderer(sp<Widget> self)
{
    return sp<RendererWidget>::make(std::move(self));
}

}
}
}
