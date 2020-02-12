#include "dear-imgui/widget/widget_with_visibility.h"

#include "core/inf/variable.h"

namespace ark {
namespace plugin {
namespace dear_imgui {

WidgetWithVisibility::WidgetWithVisibility(const sp<Widget>& delegate, sp<Boolean> visible)
    : Delegate(delegate), _visible(std::move(visible))
{
}

void WidgetWithVisibility::render()
{
    if(_visible->val())
        _delegate->render();
}

}
}
}
