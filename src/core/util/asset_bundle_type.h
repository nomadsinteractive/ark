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
    static Vector<String> listAssets(const sp<AssetBundle>& self);

//  [[script::bindings::classmethod]]
    static sp<Asset> getAsset(const sp<AssetBundle>& self, const String& name);
//  [[script::bindings::classmethod]]
    static sp<AssetBundle> getBundle(const sp<AssetBundle>& self, const String& path);

//  [[script::bindings::classmethod]]
    static Optional<String> getString(const sp<AssetBundle>& self, const String& filepath);
//  [[script::bindings::classmethod]]
    static bytearray getByteArray(const sp<AssetBundle>& self, const String& filepath);
//  [[script::bindings::classmethod]]
    static String getRealPath(const sp<AssetBundle>& self, const String& filepath);

    static sp<AssetBundle> createBuiltInAssetBundle();
    static sp<AssetBundle> createAssetBundle(const String& filepath);
};

}
