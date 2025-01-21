#pragma once

#include "core/forwarding.h"

namespace ark {

class AssetBundle {
public:
    virtual ~AssetBundle() = default;

    virtual sp<Asset> getAsset(const String& path) = 0;
    virtual sp<AssetBundle> getBundle(const String& path) = 0;

    virtual Vector<sp<Asset>> listAssets(const String& regex) = 0;
};

}
