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
    StringBundleYAML(sp<AssetBundle> assetBundle);

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

    class Node {
    public:
        Node() = default;
        DEFAULT_COPY_AND_ASSIGN(Node);

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

    struct Directory {
        Directory(sp<AssetBundle> assetBundle = nullptr);

        sp<Node> findNode(const String& resid);
        void setNode(const std::vector<String>& keys, sp<Node> node);

        sp<AssetBundle> _asset_bundle;
        std::map<String, sp<Node>> _nodes;
        std::map<String, sp<Directory>> _sub_directories;
    };

    static sp<Directory> loadAssetDirectory(Asset& asset, sp<AssetBundle> assetBundle);

private:
    Directory _root;

};

}

#endif
