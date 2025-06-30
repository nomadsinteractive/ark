#pragma once

#include "core/forwarding.h"
#include "core/base/string.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[script::bindings::class("AssetBundle")]]
class ARK_API AssetBundleType final {
public:

//  [[script::bindings::classmethod]]
    static Vector<sp<Asset>> listAssets(const sp<AssetBundle>& self, const String& regex = "");

//  [[script::bindings::classmethod]]
    static sp<Asset> get(const sp<AssetBundle>& self, const String& name);
//  [[script::bindings::classmethod]]
    static sp<AssetBundle> getBundle(const sp<AssetBundle>& self, const String& path);

//  [[script::bindings::classmethod]]
    static Optional<String> getString(const sp<AssetBundle>& self, const String& filepath);
//  [[script::bindings::classmethod]]
    static bytearray getByteArray(const sp<AssetBundle>& self, const String& filepath);
//  [[script::bindings::classmethod]]
    static String getRealPath(const sp<AssetBundle>& self, const String& filepath);

    static sp<AssetBundle> createBuiltInAssetBundle(const String& assetDir, const String& appDir);
    static sp<AssetBundle> createAssetBundle(const String& filepath);

//  [[plugin::builder::by-value("file")]]
    class FILE_DICTIONARY final : public Builder<AssetBundle> {
    public:
        FILE_DICTIONARY(BeanFactory& factory, const String& src);

        sp<AssetBundle> build(const Scope& args) override;

    private:
        String _src;
    };

//  [[plugin::builder::by-value("external")]]
    class EXTERNAL_DICTIONARY final : public Builder<AssetBundle> {
    public:
        EXTERNAL_DICTIONARY(BeanFactory& factory, const String& src);

        sp<AssetBundle> build(const Scope& args) override;

    private:
        String _src;
    };
};

}
