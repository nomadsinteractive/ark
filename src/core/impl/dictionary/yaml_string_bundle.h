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

    class Item {
    public:
        Item() = default;
        DEFAULT_COPY_AND_ASSIGN(Item);

        void setValue(String value);
        void addSequenceValue(String value);

        void makeSequence();
        bool isSequence() const;

        const sp<String>& value() const;
        const sp<std::vector<sp<String>>>& sequence() const;

    private:
        sp<String> _value;
        sp<std::vector<sp<String>>> _sequence;
    };

    void loadBundle(const String& name);

    sp<Item>& makeKey(std::map<String, sp<Item>>& bundle, const std::vector<String>& keys) const;

private:
    sp<AssetBundle> _resource;
    std::map<String, std::map<String, sp<Item>>> _bundle;
};

}

#endif
