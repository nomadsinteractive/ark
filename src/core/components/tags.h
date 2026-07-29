#pragma once

#include "core/base/api.h"
#include "core/collection/args.h"
#include "core/types/box.h"
#include "core/inf/debris.h"

namespace ark {

//[[script::bindings::debris]]
class ARK_API Tags final : public Debris {
public:
//  [[script::bindings::auto]]
    Tags(Box tag = {}, Args tags = {});

//  [[script::bindings::property]]
    Box tag() const;

//  [[script::bindings::auto]]
    void addTag(TypeId typeId);
//  [[script::bindings::auto]]
    bool hasTag(TypeId typeId) const;
//  [[script::bindings::auto]]
    void removeTag(TypeId typeId);

//  [[script::bindings::map(get)]]
    Box getTag(TypeId typeId) const;
//  [[script::bindings::map(set)]]
    void setTag(TypeId typeId, Box tag);
//  [[script::bindings::map(del)]]
    void delTag(TypeId typeId);

    void traverse(const Visitor& visitor) override;

private:
    Map<TypeId, Box> _tags;
};

}
