#pragma once

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/base/timestamp.h"
#include "core/inf/variable.h"

namespace ark {

//[[script::bindings::extends(Integer)]]
class ARK_API NamedType : public Integer {
public:
    NamedType(String name);
    NamedType(int32_t type);

    bool update(uint64_t timestamp) override;
    int val() override;

    const String& name() const;
    int32_t type() const;

//  [[script::bindings::auto]]
    static String reverse(int32_t type);

private:
    String _name;
    int32_t _type;
    Timestamp _timestamp;
};

}
