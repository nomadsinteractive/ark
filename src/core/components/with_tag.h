#pragma once

#include "core/base/api.h"
#include "core/inf/debris.h"
#include "core/inf/wirable.h"
#include "core/components/with_debris.h"

#include "core/types/box.h"

namespace ark {

//[[script::bindings::debris]]
class ARK_API WithTag final : public Wirable, public Debris {
public:
//  [[script::bindings::auto]]
    WithTag(Box tag);

    void onWire(const WiringContext& context, const Box& self) override;
    void traverse(const Visitor& visitor) override;

//  [[script::bindings::property]]
    const Box& tag() const;
//  [[script::bindings::property]]
    void setTag(Box tag);

private:
    Box _tag;
    WithDebris _with_debris;
};

}
