#pragma once

#include <map>

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/inf/dictionary.h"
#include "core/types/box.h"
#include "core/types/shared_ptr.h"

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
    Scope(std::map<String, Box> variables, sp<Queries> queries = nullptr);
    DEFAULT_COPY_AND_ASSIGN(Scope);

    template<typename T> sp<T> build(const String& name, const Scope& args) const;

    Box get(const String& name) override;

    void put(const String& name, Box value);

    const std::map<String, Box>& variables() const;

    Box getObject(const String& name) const;

private:
    std::map<String, Box> _variables;
    sp<Queries> _queries;
};

}
