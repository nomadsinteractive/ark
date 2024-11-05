#pragma once

#include "core/base/api.h"
#include "core/inf/debris.h"
#include "core/inf/wirable.h"
#include "core/traits/with_debris.h"

#include "core/types/box.h"

namespace ark {

//[[script::bindings::debris]]
class ARK_API WithTag final : public Wirable, public Debris {
public:
//  [[script::bindings::auto]]
    WithTag(Box tag);

    TypeId onPoll(WiringContext& context) override;
    void onWire(const WiringContext& context) override;
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
