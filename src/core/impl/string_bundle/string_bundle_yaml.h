#ifndef ARK_CORE_IMPL_STRING_BUNDLE_STRING_BUNDLE_YAML_H_
#define ARK_CORE_IMPL_STRING_BUNDLE_STRING_BUNDLE_YAML_H_

#include <map>

#include "core/types/shared_ptr.h"

#include "core/inf/builder.h"
#include "core/inf/string_bundle.h"
#include "core/forwarding.h"

namespace ark {

class StringBundleYAML : public StringBundle {
public:
    StringBundleYAML(const sp<AssetBundle>& resource);

    virtual sp<String> getString(const String& resid) override;
    virtual std::vector<String> getStringArray(const String& resid) override;

//  [[plugin::builder("yaml")]]
    class BUILDER : public Builder<StringBundle> {
    public:
        BUILDER(BeanFactory& factory, const document& doc);

        virtual sp<StringBundle> build(const Scope& args) override;

    private:
        sp<Builder<String>> _src;
    };

private:

    class Item {
    public:
        Item() = default;
        DEFAULT_COPY_AND_ASSIGN(Item);

        void setValue(String value);
        void addSequenceValue(String value);

        void makeSequence();
        bool isSequence() const;

        const sp<String>& value() const;
        const sp<std::vector<String>>& sequence() const;

    private:
        sp<String> _value;
        sp<std::vector<String>> _sequence;
    };

    void loadBundle(const String& name);

    const std::map<String, sp<Item>>& getPackageBundle(const String& resid, String& nodename);
    sp<Item>& makeKey(std::map<String, sp<Item>>& bundle, const std::vector<String>& keys) const;

private:
    sp<AssetBundle> _resource;
    std::map<String, std::map<String, sp<Item>>> _bundle;

};

}

#endif
