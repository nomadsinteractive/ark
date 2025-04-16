#pragma once

#include "core/base/api.h"
#include "core/inf/debris.h"

namespace ark {

class ARK_API Tags final : public Debris {
public:
//  [[script::bindings::auto]]
    Tags();

//  [[script::bindings::auto]]
    void setTag(uint64_t typeId, Box tag);
//  [[script::bindings::auto]]
    Box getTag(uint64_t typeId) const;

    void traverse(const Visitor& visitor) override;

private:
    Map<uint64_t, Box> _tags;
};

}
