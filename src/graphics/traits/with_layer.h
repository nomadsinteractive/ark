#pragma once

#include "core/base/api.h"
#include "core/inf/wirable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API WithLayer : public Wirable {
public:
//  [[script::bindings::auto]]
    WithLayer(sp<Layer> layer);

    void onWire(const Traits& components) override;

private:
    sp<Layer> _layer;
};

}
