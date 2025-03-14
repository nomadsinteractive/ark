#pragma once

#include "core/base/api.h"
#include "core/inf/debris.h"

namespace ark {

class ARK_API Crate final : public Debris {
public:
//  [[script::bindings::auto]]
    Crate();

//  [[script::bindings::auto]]
    void setComponent(uint64_t typeId, Box component);
//  [[script::bindings::auto]]
    Box getComponent(uint64_t typeId) const;

    void traverse(const Visitor& visitor) override;

private:
    Map<uint64_t, Box> _components;
};

}

