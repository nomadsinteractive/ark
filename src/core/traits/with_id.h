#pragma once

#include "core/inf/wirable.h"

namespace ark {

class ARK_API WithId final : public Wirable {
public:
//  [[script::bindings::auto]]
    WithId();

    TypeId onPoll(WiringContext& context) override;
    void onWire(const WiringContext& context) override;

//  [[script::bindings::auto]]
    RefId id() const;

private:
    RefId _id;

    friend class Entity;
};

}