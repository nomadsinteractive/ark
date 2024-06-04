#pragma once

#include "core/inf/wirable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "layout_param.h"

namespace ark {

class ARK_API Label final : public Wirable {
public:
//  [[script::bindings::auto]]
    Label(sp<Text> text, sp<LayoutParam> layoutParam);

    std::vector<std::pair<TypeId, Box>> onWire(const Traits& components) override;

private:
    sp<Text> _text;
    sp<LayoutParam> _layout_param;
};

}
