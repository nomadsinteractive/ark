#pragma once

#include "core/forwarding.h"
#include "core/base/wrapper.h"

#include "dear-imgui/inf/widget.h"

namespace ark::plugin::dear_imgui {

class WidgetWithVisibility : public Wrapper<Widget>, public Widget {
public:
    WidgetWithVisibility(sp<Widget> delegate, sp<Boolean> visible);

    void render() override;

private:
    sp<Boolean> _visible;
};

}
