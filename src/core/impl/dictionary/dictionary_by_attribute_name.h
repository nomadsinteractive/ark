#pragma once

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "core/inf/dictionary.h"

namespace ark {

class DictionaryByAttributeName final : public Dictionary<document> {
public:
    DictionaryByAttributeName(const document& doc, const String& attrName, const String& childName = String());

    document get(const String& name) override;

private:
    Map<String, document> _by_names;
};

}
