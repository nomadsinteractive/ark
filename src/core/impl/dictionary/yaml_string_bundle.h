#ifndef ARK_CORE_IMPL_DIRECTORY_YAML_STRING_BUNDLE_H_
#define ARK_CORE_IMPL_DIRECTORY_YAML_STRING_BUNDLE_H_

#include <map>

#include "core/types/shared_ptr.h"

#include "core/inf/builder.h"
#include "core/inf/dictionary.h"
#include "core/forwarding.h"

namespace ark {

class YAMLStringBundle : public StringBundle {
public:
    YAMLStringBundle(const sp<AssetBundle>& resource);

    virtual sp<String> get(const String& name);

//  [[plugin::builder("yaml")]]
    class BUILDER : public Builder<StringBundle> {
    public:
        BUILDER(BeanFactory& factory, const document& doc);

        virtual sp<StringBundle> build(const Scope& args);

    private:
        sp<Builder<String>> _src;
    };

private:
    void loadBundle(const String& name);

private:
    sp<AssetBundle> _resource;
    std::map<String, std::map<String, sp<String>>> _bundle;
};

}

#endif
