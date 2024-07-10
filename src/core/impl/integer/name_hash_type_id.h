#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/string.h"
#include "core/inf/variable.h"

namespace ark {

class ARK_API NameHashTypeId final : public Integer {
public:
    NameHashTypeId(String name);

    bool update(uint64_t timestamp) override;
    int32_t val() override;

    const String& name() const;

private:
    String _name;
    int32_t _name_hash;
};

}
