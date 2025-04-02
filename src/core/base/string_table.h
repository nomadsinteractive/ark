#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API StringTable {
public:

    void addStringBundle(const String& name, const sp<StringBundle>& stringTable);
    sp<StringBundle> getStringBundle(const String& name) const;

    Optional<String> getString(const String& stringTableName, const String& stringName, bool alert) const;
    Optional<String> getString(const String& name, bool alert) const;

    Vector<String> getStringArray(const String& stringTableName, const String& name, bool alert) const;
    Vector<String> getStringArray(const String& name, bool alert) const;

private:
    Map<String, sp<StringBundle>> _string_bundle_by_name;
};

}
