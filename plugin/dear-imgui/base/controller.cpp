#include "dear-imgui/base/controller.h"

#include "dear-imgui/widget/widget_with_visibility.h"

namespace ark {
namespace plugin {
namespace dear_imgui {

Controller::Controller(sp<Widget> widget)
    : _delegate(widget), _widget(std::move(widget)) {
}

void Controller::render()
{
    _delegate->render();
}

const sp<Boolean>& Controller::visible() const
{
    return _visible;
}

void Controller::setVisible(const sp<Boolean>& visible)
{
    _visible = visible;
    _delegate = visible ? sp<Widget>::make<WidgetWithVisibility>(_widget, visible) : _widget;
}

}
}
}
