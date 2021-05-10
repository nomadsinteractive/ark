#ifndef ARK_CORE_IMPL_STRING_BUNDLE_STRING_BUNDLE_IMPL_H_
#define ARK_CORE_IMPL_STRING_BUNDLE_STRING_BUNDLE_IMPL_H_

#include <map>

#include "core/types/shared_ptr.h"
#include "core/inf/builder.h"
#include "core/inf/string_bundle.h"


namespace ark {

class StringBundleImpl : public StringBundle {
public:
    StringBundleImpl(std::map<String, sp<StringBundle>> directories);

    virtual sp<String> getString(const String& resid) override;
    virtual std::vector<String> getStringArray(const String& resid) override;

//  [[plugin::builder]]
    class BUILDER : public Builder<StringBundle> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<StringBundle> build(const Scope& args) override;

    private:
        std::map<String, sp<Builder<StringBundle>>> _directories;
    };

private:
    sp<StringBundle> split(const String& resid, String& remains);

private:
    std::map<String, sp<StringBundle>> _directories;
};

}

#endif
