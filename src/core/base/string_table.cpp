#include "core/base/string_table.h"

#include "core/base/string.h"
#include "core/inf/string_bundle.h"

namespace ark {

namespace {

class MergedStringBundle final : public StringBundle {
public:
    MergedStringBundle(sp<StringBundle> d1, sp<StringBundle> d2)
        : _d1(std::move(d1)), _d2(std::move(d2)) {
    }

    Optional<String> getString(const String& name) override {
        Optional<String> value = _d1->getString(name);
        return value ? value : _d2->getString(name);
    }

    Vector<String> getStringArray(const String& resid) override {
        Vector<String> sa = _d1->getStringArray(resid);
        return sa.size() ? sa : _d2->getStringArray(resid);
    }

private:
    sp<StringBundle> _d1;
    sp<StringBundle> _d2;
};

}

void StringTable::addStringBundle(const String& name, const sp<StringBundle>& stringTable)
{
    ASSERT(stringTable);
    if(const auto iter = _string_bundle_by_name.find(name); iter != _string_bundle_by_name.end())
        _string_bundle_by_name[name] = sp<MergedStringBundle>::make(iter->second, stringTable);
    else
        _string_bundle_by_name[name] = stringTable;
}

sp<StringBundle> StringTable::getStringBundle(const String& name) const
{
    const auto iter = _string_bundle_by_name.find(name);
    return iter != _string_bundle_by_name.end() ? iter->second : nullptr;
}

Optional<String> StringTable::getString(const String& stringTableName, const String& stringName, const bool alert) const
{
    const sp<StringBundle> sb = getStringBundle(stringTableName);
    CHECK(!alert || sb, "String bundle \"%s\" does not exist", stringTableName.c_str());
    Optional<String> str = sb ? sb->getString(stringName) : Optional<String>();
    CHECK(!alert || str, "String bundle \"%s\" has no resource \"%s\"", stringTableName.c_str(), stringName.c_str());
    return str;
}

Optional<String> StringTable::getString(const String& name, bool alert) const
{
    const auto pos = name.find('/');
    CHECK(pos != String::npos, "The name \"%s\" doest follow [stringtablename/stringname] pattern", name.c_str());
    return getString(name.substr(0, pos), name.substr(pos + 1), alert);
}

Vector<String> StringTable::getStringArray(const String& stringTableName, const String& name, const bool alert) const
{
    const sp<StringBundle> sb = getStringBundle(stringTableName);
    CHECK(!alert || sb, "String bundle \"%s\" does not exist", stringTableName.c_str());
    if(sb)
        return sb->getStringArray(name);
    DCHECK(!alert, "String bundle \"%s\" has no resource \"%s\"", stringTableName.c_str(), name.c_str());
    return {};
}

Vector<String> StringTable::getStringArray(const String& name, const bool alert) const
{
    auto pos = name.find('/');
    DCHECK(pos != String::npos, "The name \"%s\" doest follow [stringtablename/stringname] pattern", name.c_str());
    return getStringArray(name.substr(0, pos), name.substr(pos + 1), alert);
}

}
