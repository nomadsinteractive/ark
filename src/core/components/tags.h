#pragma once

#include "core/base/api.h"
#include "core/types/box.h"
#include "core/inf/debris.h"

namespace ark {

//[[script::bindings::debris]]
class ARK_API Tags final : public Debris {
public:
//  [[script::bindings::auto]]
    Tags(Box tag = {}, Vector<Box> tags = {});

//  [[script::bindings::property]]
    Box tag() const;

//  [[script::bindings::map(get)]]
    Box getTag(TypeId typeId) const;
//  [[script::bindings::map(set)]]
    void setTag(TypeId typeId, Box tag);
//  [[script::bindings::map(del)]]
    void removeTag(TypeId typeId);

    void traverse(const Visitor& visitor) override;

private:
    Map<TypeId, Box> _tags;
};

}
