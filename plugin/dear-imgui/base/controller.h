#ifndef ARK_PLUGIN_DEAR_IMGUI_BASE_CONTROLLER_H_
#define ARK_PLUGIN_DEAR_IMGUI_BASE_CONTROLLER_H_

#include "core/types/shared_ptr.h"

#include "dear-imgui/forwarding.h"
#include "dear-imgui/api.h"

#include "dear-imgui/inf/widget.h"

namespace ark {
namespace plugin {
namespace dear_imgui {

//[[script::bindings::extends(Widget)]]
class ARK_PLUGIN_DEAR_IMGUI_API Controller : public Widget {
public:
    Controller(sp<Widget> widget);

    virtual void render() override;

//[[script::bindings::property]]
    const sp<Boolean>& visible() const;
//[[script::bindings::property]]
    void setVisible(const sp<Boolean>& visible);

private:
    sp<Widget> _delegate;
    sp<Widget> _widget;

    sp<Boolean> _visible;

};

}
}
}

#endif
