#pragma once

#include "core/inf/wirable.h"

namespace ark {

class ARK_API WithId final : public Wirable {
public:
//  [[script::bindings::auto]]
    WithId();

    void onWire(const WiringContext& context, const Box& self) override;

//  [[script::bindings::auto]]
    RefId id() const;

private:
    RefId _id;

    friend class Entity;
};

}