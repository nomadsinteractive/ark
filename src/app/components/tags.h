#pragma once

#include "core/base/api.h"
#include "core/inf/debris.h"

namespace ark {

class ARK_API Tags final : public Debris {
public:
//  [[script::bindings::auto]]
    Tags() = default;

//  [[script::bindings::auto]]
    void setTag(Box tag, uint64_t typeId);
//  [[script::bindings::auto]]
    Box getTag(uint64_t typeId = 0) const;

    void traverse(const Visitor& visitor) override;

private:
    Map<uint64_t, Box> _tags;
};

}
