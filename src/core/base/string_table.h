#ifndef ARK_CORE_BASE_STRING_TABLE_H_
#define ARK_CORE_BASE_STRING_TABLE_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API StringTable {
public:

    void addStringBundle(const String& name, const sp<StringBundle>& stringTable);
    const sp<StringBundle>& getStringBundle(const String& name);

    sp<String> getString(const String& stringTableName, const String& stringName);
    sp<String> getString(const String& name);

private:
    std::map<String, sp<StringBundle>> _string_bundle_by_name;

};

}

#endif
