#ifndef ARK_PLUGIN_DEAR_IMGUI_WIDGET_WIDGET_WITH_VISIBILITY_H_
#define ARK_PLUGIN_DEAR_IMGUI_WIDGET_WIDGET_WITH_VISIBILITY_H_

#include "core/forwarding.h"
#include "core/base/delegate.h"

#include "dear-imgui/inf/widget.h"

namespace ark {
namespace plugin {
namespace dear_imgui {

class WidgetWithVisibility : public Delegate<Widget>, public Widget {
public:
    WidgetWithVisibility(sp<Widget> delegate, sp<Boolean> visible);

    virtual void render() override;

private:
    sp<Boolean> _visible;
};

}
}
}

#endif
