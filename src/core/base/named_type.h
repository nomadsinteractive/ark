#pragma once

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/base/timestamp.h"

namespace ark {

class ARK_API NamedType  {
public:
    NamedType(String name);
    NamedType(int32_t type);

    const String& name() const;
    int32_t type() const;

    static String reverse(int32_t type);

private:
    String _name;
    int32_t _type;
    Timestamp _timestamp;
};

}
