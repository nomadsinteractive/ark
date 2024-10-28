#pragma once

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/base/timestamp.h"
#include "core/inf/variable.h"

namespace ark {

//[[script::bindings::extends(Integer)]]
class ARK_API NamedType final : public Integer {
public:
//  [[script::bindings::auto]]
    NamedType(String name);
//  [[script::bindings::auto]]
    NamedType(TypeId typeId);

    bool update(uint64_t timestamp) override;
    int32_t val() override;

//  [[script::bindings::property]]
    TypeId id() const;
//  [[script::bindings::property]]
    const String& name() const;

//  [[script::bindings::auto]]
    static String reverse(int32_t type);

private:
    TypeId _id;
    String _name;
    Timestamp _timestamp;
};

}
