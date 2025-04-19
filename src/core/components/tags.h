#pragma once

#include "core/base/api.h"
#include "core/types/box.h"
#include "core/inf/debris.h"

namespace ark {

//[[script::bindings::debris]]
class ARK_API Tags final : public Debris {
public:
//  [[script::bindings::auto]]
    Tags(Box tag = {});

//  [[script::bindings::property]]
    Box tag() const;

//  [[script::bindings::map(get)]]
    Box getTag(uint64_t typeId) const;
//  [[script::bindings::map(set)]]
    void setTag(uint64_t typeId, Box tag);

    void traverse(const Visitor& visitor) override;

private:
    Map<uint64_t, Box> _tags;
};

}
