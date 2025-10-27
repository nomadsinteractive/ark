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

class WidgetWrapper final : public Widget, public Wrapper<Widget>, Implements<WidgetWrapper, Wrapper<Widget>, Widget> {
public:
    WidgetWrapper(sp<Widget> delegate)
        : Wrapper(std::move(delegate)) {
    }

    void render() override {
        if(_wrapped)
            _wrapped->render();
    }
};

class WidgetList final : public Widget {
public:
    WidgetList(Vector<sp<Widget>> widgets)
        : _widgets(std::move(widgets)) {
    }

    void render() override
    {
        for(const sp<Widget>& i : _widgets)
            i->render();
    }

private:
    Vector<sp<Widget>> _widgets;
};

}

sp<Widget> WidgetType::create(sp<Widget> delegate)
{
    return sp<Widget>::make<WidgetWrapper>(std::move(delegate));
}

sp<Widget> WidgetType::create(Vector<sp<Widget>> delegate)
{
    return sp<Widget>::make<WidgetWrapper>(sp<Widget>::make<WidgetList>(std::move(delegate)));
}

void WidgetType::visibleIf(const sp<Widget>& self, sp<Boolean> visibility)
{
    reset(self, sp<Widget>::make<WidgetWithVisibility>(self.ensureInstance<Wrapper<Widget>>()->wrapped(), std::move(visibility)));
}

void WidgetType::reset(const sp<Widget>& self, sp<Widget> wrapped)
{
    const sp<Wrapper<Widget>> wrapper = self.ensureInstance<Wrapper<Widget>>("Must be a Wrapper<Widget> instance to set its wrapped attribute");
    Ark::instance().applicationContext()->deferUnref(wrapper->reset(std::move(wrapped)));
}

}
