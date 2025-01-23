#pragma once

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/inf/dictionary.h"
#include "core/types/box.h"

namespace ark {

/*

class BoxBundle {
    [[script::bindings::getprop]]
    virtual Box get(const String& name) override;
};

*/

class ARK_API Scope final : public BoxBundle {
public:
    Scope() = default;
    Scope(Map<String, Box> variables);
    DEFAULT_COPY_AND_ASSIGN(Scope);

    Box get(const String& name) override;
    void put(const String& name, Box value);

    const Map<String, Box>& variables() const;

    Optional<Box> getObject(const String& name) const;

private:
    Map<String, Box> _variables;
};

}
