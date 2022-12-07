#include "dear-imgui/widget/widget_with_visibility.h"

#include "core/inf/variable.h"

namespace ark {
namespace plugin {
namespace dear_imgui {

WidgetWithVisibility::WidgetWithVisibility(sp<Widget> delegate, sp<Boolean> visible)
    : Wrapper(std::move(delegate)), _visible(std::move(visible))
{
}

void WidgetWithVisibility::render()
{
    if(_visible->val())
        _wrapped->render();
}

}
}
}
