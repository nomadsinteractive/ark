#pragma once

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/inf/variable.h"

namespace ark {

//[[script::bindings::extends(Integer)]]
class ARK_API NamedHash final : public Integer {
public:
//  [[script::bindings::auto]]
    NamedHash(String value);
//  [[script::bindings::auto]]
    NamedHash(HashId value);

    explicit operator bool() const;

//  [[script::bindings::operator(index)]]
//  [[script::bindings::property]]
    HashId hash() const;
//  [[script::bindings::property]]
    const String& name() const;

    bool update(uint32_t tick) override;
    int32_t val() override;

//  [[script::bindings::auto]]
    static const String& reverse(HashId hash);

private:
    HashId _hash;
    String _name;
};

}
