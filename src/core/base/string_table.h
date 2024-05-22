#pragma once

#include <map>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API StringTable {
public:

    void addStringBundle(const String& name, const sp<StringBundle>& stringTable);
    const sp<StringBundle>& getStringBundle(const String& name);

    sp<String> getString(const String& stringTableName, const String& stringName, bool alert);
    sp<String> getString(const String& name, bool alert);

    std::vector<String> getStringArray(const String& stringTableName, const String& name, bool alert);
    std::vector<String> getStringArray(const String& name, bool alert);

private:
    std::map<String, sp<StringBundle>> _string_bundle_by_name;

};

}
