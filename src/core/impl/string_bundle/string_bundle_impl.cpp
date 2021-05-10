#include "core/impl/string_bundle/string_bundle_impl.h"

#include "core/base/bean_factory.h"
#include "core/util/documents.h"
#include "core/util/strings.h"

namespace ark {

StringBundleImpl::StringBundleImpl(std::map<String, sp<StringBundle>> directories)
    : _directories(std::move(directories))
{
}

sp<String> StringBundleImpl::getString(const String& resid)
{
    String remains;
    const sp<StringBundle> sb = split(resid, remains);
    return sb ? sb->getString(remains) : nullptr;
}

std::vector<String> StringBundleImpl::getStringArray(const String& resid)
{
    String remains;
    const sp<StringBundle> sb = split(resid, remains);
    return sb ? sb->getStringArray(remains) : std::vector<String>();
}

sp<StringBundle> StringBundleImpl::split(const String& resid, String& remains)
{
    String dirname, filename;
    Strings::cut(resid, dirname, filename, '/');

    const auto iter = _directories.find(dirname);
    if(iter != _directories.end())
    {
        remains = filename;
        return iter->second;
    }

    const auto defaultIter = _directories.find(".");
    if(defaultIter != _directories.end()) {
        remains = resid;
        return defaultIter->second;
    }

    return nullptr;
}

StringBundleImpl::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
{
    for(const document& i : manifest->children("directory"))
        _directories.insert(std::make_pair(Documents::getAttribute(i, Constants::Attributes::NAME, "."), factory.ensureBuilder<StringBundle>(i)));
}

sp<StringBundle> StringBundleImpl::BUILDER::build(const Scope& args)
{
    std::map<String, sp<StringBundle>> directories;
    for(const auto& i : _directories)
        directories.insert(std::make_pair(i.first, i.second->build(args)));
    return sp<StringBundle>::make<StringBundleImpl>(std::move(directories));
}

}
