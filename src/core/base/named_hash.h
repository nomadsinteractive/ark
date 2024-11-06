#pragma once

#include "core/base/api.h"
#include "core/base/string.h"

namespace ark {

class ARK_API NamedHash final {
public:
//  [[script::bindings::auto]]
    NamedHash(String name);
//  [[script::bindings::auto]]
    NamedHash(HashId hash);

//  [[script::bindings::operator(index)]]
//  [[script::bindings::property]]
    HashId hash() const;
//  [[script::bindings::property]]
    const String& name() const;

//  [[script::bindings::auto]]
    static String reverse(HashId hash);

private:
    HashId _hash;
    String _name;
};

}
