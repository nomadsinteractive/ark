#include "core/base/string_table.h"

#include "core/base/string.h"
#include "core/inf/dictionary.h"

namespace ark {

namespace {

class MergedStringBundle : public StringBundle {
public:
    MergedStringBundle(const sp<Dictionary<sp<String>>>& d1, const sp<Dictionary<sp<String>>>& d2)
        : _d1(d1), _d2(d2) {
    }

    virtual sp<String> get(const String& name) override {
        const sp<String> value = _d1->get(name);
        return value ? value : _d2->get(name);
    }

private:
    sp<Dictionary<sp<String>>> _d1;
    sp<Dictionary<sp<String>>> _d2;
};

}

void StringTable::addStringBundle(const String& name, const sp<StringBundle>& stringTable)
{
    const auto iter = _string_bundle_by_name.find(name);
    if(iter != _string_bundle_by_name.end())
        _string_bundle_by_name[name] = sp<MergedStringBundle>::make(iter->second, stringTable);
    else
        _string_bundle_by_name[name] = stringTable;
}

const sp<StringBundle>& StringTable::getStringBundle(const String& name)
{
    auto iter = _string_bundle_by_name.find(name);
    DCHECK(iter != _string_bundle_by_name.end(), "String table \"%s\" does not exists", name.c_str());
    return iter->second;
}

sp<String> StringTable::getString(const String& stringTableName, const String& stringName)
{
    const sp<String> str = getStringBundle(stringTableName)->get(stringName);
    DCHECK(str, "String resource \"%s/%s\" does not exists", stringTableName.c_str(), stringName.c_str());
    return str;
}

sp<String> StringTable::getString(const String& name)
{
    auto pos = name.find('/');
    DCHECK(pos != String::npos, "The name \"%s\" doest follow [stringtablename/stringname] pattern", name.c_str());
    return getString(name.substr(0, pos), name.substr(pos + 1));
}

}
