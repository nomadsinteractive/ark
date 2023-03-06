#include "core/base/string_table.h"

#include "core/base/string.h"
#include "core/inf/string_bundle.h"

namespace ark {

namespace {

class MergedStringBundle : public StringBundle {
public:
    MergedStringBundle(const sp<StringBundle>& d1, const sp<StringBundle>& d2)
        : _d1(d1), _d2(d2) {
    }

    virtual sp<String> getString(const String& name) override {
        const sp<String> value = _d1->getString(name);
        return value ? value : _d2->getString(name);
    }

    virtual std::vector<String> getStringArray(const String& resid) override {
        std::vector<String> sa = _d1->getStringArray(resid);
        return sa.size() ? sa : _d2->getStringArray(resid);
    }

private:
    sp<StringBundle> _d1;
    sp<StringBundle> _d2;
};

}

void StringTable::addStringBundle(const String& name, const sp<StringBundle>& stringTable)
{
    DASSERT(stringTable);
    const auto iter = _string_bundle_by_name.find(name);
    if(iter != _string_bundle_by_name.end())
        _string_bundle_by_name[name] = sp<MergedStringBundle>::make(iter->second, stringTable);
    else
        _string_bundle_by_name[name] = stringTable;
}

const sp<StringBundle>& StringTable::getStringBundle(const String& name)
{
    const auto iter = _string_bundle_by_name.find(name);
    return iter != _string_bundle_by_name.end() ? iter->second : sp<StringBundle>::null();
}

sp<String> StringTable::getString(const String& stringTableName, const String& stringName, bool alert)
{
    const sp<StringBundle>& sb = getStringBundle(stringTableName);
    CHECK(!alert || sb, "String bundle \"%s\" does not exist", stringTableName.c_str());
    const sp<String> str = sb ? sb->getString(stringName) : sp<String>::null();
    CHECK(!alert || str, "String bundle \"%s\" has no resource \"%s\"", stringTableName.c_str(), stringName.c_str());
    return str;
}

sp<String> StringTable::getString(const String& name, bool alert)
{
    auto pos = name.find('/');
    CHECK(pos != String::npos, "The name \"%s\" doest follow [stringtablename/stringname] pattern", name.c_str());
    return getString(name.substr(0, pos), name.substr(pos + 1), alert);
}

std::vector<String> StringTable::getStringArray(const String& stringTableName, const String& name, bool alert)
{
    const sp<StringBundle>& sb = getStringBundle(stringTableName);
    CHECK(!alert || sb, "String bundle \"%s\" does not exist", stringTableName.c_str());
    if(sb)
        return sb->getStringArray(name);
    DCHECK(!alert, "String bundle \"%s\" has no resource \"%s\"", stringTableName.c_str(), name.c_str());
    return {};
}

std::vector<String> StringTable::getStringArray(const String& name, bool alert)
{
    auto pos = name.find('/');
    DCHECK(pos != String::npos, "The name \"%s\" doest follow [stringtablename/stringname] pattern", name.c_str());
    return getStringArray(name.substr(0, pos), name.substr(pos + 1), alert);
}

}
