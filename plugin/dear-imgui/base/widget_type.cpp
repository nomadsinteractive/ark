#include "dear-imgui/base/widget_type.h"

#include "core/ark.h"
#include "core/base/wrapper.h"
#include "core/types/implements.h"

#include "graphics/inf/renderer.h"

#include "app/base/application_context.h"

#include "dear-imgui/widget/widget_with_visibility.h"

namespace ark::plugin::dear_imgui {

namespace {

class RendererWidget final : public Renderer {
public:
    RendererWidget(sp<Widget> widget)
        : _widget(std::move(widget)) {
    }

    void render(RenderRequest& /*renderRequest*/, const V3& /*position*/, const sp<DrawDecorator>& /*drawDecorator*/) override {
        _widget->render();
    }

private:
    sp<Widget> _widget;
};

class WidgetWrapper final : public Widget, public Wrapper<Widget>, public Implements<WidgetWrapper, Wrapper<Widget>, Widget> {
public:
    WidgetWrapper(sp<Widget> delegate)
        : Wrapper(std::move(delegate)) {
    }

    void render() override {
        if(_wrapped)
            _wrapped->render();
    }
};

class WidgetBefore final : public Widget, public Wrapper<Widget>, public Implements<WidgetBefore, Wrapper<Widget>, Widget> {
public:
    WidgetBefore(sp<Widget> before, sp<Widget> after)
        : Wrapper(std::move(after)), _before(std::move(before)) {
    }

    void render() override {
        _before->render();
        if(_wrapped)
            _wrapped->render();
    }

private:
    sp<Widget> _before;
};

}

sp<Widget> WidgetType::create(sp<Widget> wrapped)
{
    return sp<Widget>::make<WidgetWrapper>(std::move(wrapped));
}

sp<Widget> WidgetType::makeVisible(sp<Widget> self, sp<Boolean> visibility)
{
    return sp<Widget>::make<WidgetWithVisibility>(std::move(self), std::move(visibility));
}

sp<Widget> WidgetType::before(sp<Widget> self, sp<Widget> after)
{
    return sp<Widget>::make<WidgetBefore>(std::move(self), std::move(after));
}

void WidgetType::reset(const sp<Widget>& self, sp<Widget> wrapped)
{
    const sp<Wrapper<Widget>> wrapper = self.ensureInstance<Wrapper<Widget>>("Must be a Wrapper<Widget> instance to set its wrapped attribute");
    Ark::instance().applicationContext()->deferUnref(wrapper->reset(std::move(wrapped)));
}

sp<Renderer> WidgetType::toRenderer(sp<Widget> self)
{
    return sp<Renderer>::make<RendererWidget>(std::move(self));
}

}
