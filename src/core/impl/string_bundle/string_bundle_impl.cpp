#include "core/impl/string_bundle/string_bundle_impl.h"

#include "core/base/bean_factory.h"
#include "core/base/constants.h"
#include "core/util/documents.h"

namespace ark {

StringBundleImpl::StringBundleImpl(Map<String, sp<StringBundle>> directories)
    : _directories(std::move(directories))
{
}

sp<String> StringBundleImpl::getString(const String& resid)
{
    String remains;
    const sp<StringBundle> sb = split(resid, remains);
    return sb ? sb->getString(remains) : nullptr;
}

Vector<String> StringBundleImpl::getStringArray(const String& resid)
{
    String remains;
    const sp<StringBundle> sb = split(resid, remains);
    return sb ? sb->getStringArray(remains) : Vector<String>();
}

sp<StringBundle> StringBundleImpl::split(const String& resid, String& remains)
{
    auto [dirname, filename] = resid.cut('/');

    ASSERT(filename);
    if(const auto iter = _directories.find(dirname); iter != _directories.end())
    {
        remains = std::move(filename.value());
        return iter->second;
    }

    if(const auto defaultIter = _directories.find("."); defaultIter != _directories.end()) {
        remains = resid;
        return defaultIter->second;
    }

    return nullptr;
}

StringBundleImpl::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
{
    for(const document& i : manifest->children("directory"))
        _directories.insert(std::make_pair(Documents::getAttribute(i, constants::NAME, "."), factory.ensureBuilder<StringBundle>(i)));
}

sp<StringBundle> StringBundleImpl::BUILDER::build(const Scope& args)
{
    Map<String, sp<StringBundle>> directories;
    for(const auto& [k, v] : _directories)
        directories.insert(std::make_pair(k, v->build(args)));
    return sp<StringBundle>::make<StringBundleImpl>(std::move(directories));
}

}
