#include "dear-imgui/base/widget_type.h"

#include "graphics/inf/renderer.h"

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
}

sp<Widget> WidgetType::makeVisible(sp<Widget> self, sp<Boolean> visibility)
{
    return sp<WidgetWithVisibility>::make(self, visibility);
}

sp<Renderer> WidgetType::toRenderer(sp<Widget> self)
{
    return sp<RendererWidget>::make(std::move(self));
}

}
}
}
