#pragma once

#include "core/types/shared_ptr.h"

#include "core/inf/builder.h"
#include "core/inf/string_bundle.h"
#include "core/forwarding.h"

namespace ark {

class StringBundleYAML final : public StringBundle {
public:
    StringBundleYAML(sp<AssetBundle> assetBundle);

    sp<String> getString(const String& resid) override;
    Vector<String> getStringArray(const String& resid) override;

//  [[plugin::builder("yaml")]]
    class BUILDER final : public Builder<StringBundle> {
    public:
        BUILDER(BeanFactory& factory, const document& doc);

        sp<StringBundle> build(const Scope& args) override;

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
        const sp<Vector<String>>& sequence() const;

    private:
        sp<String> _value;
        sp<Vector<String>> _sequence;
    };

    struct Directory {
        Directory(sp<AssetBundle> assetBundle = nullptr);

        sp<Node> findNode(const String& resid);
        void setNode(const Vector<String>& keys, sp<Node> node);

        sp<AssetBundle> _asset_bundle;
        Map<String, sp<Node>> _nodes;
        Map<String, sp<Directory>> _sub_directories;
    };

    static sp<Directory> loadAssetDirectory(Asset& asset, sp<AssetBundle> assetBundle);

private:
    Directory _root;
};

}
