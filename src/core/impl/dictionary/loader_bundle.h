#pragma once

#include "core/base/string.h"
#include "core/inf/asset.h"
#include "core/inf/asset_bundle.h"
#include "core/inf/dictionary.h"
#include "core/inf/loader.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class LoaderBundle final : public Dictionary<T> {
private:
    typedef Loader<T> LoaderType;

public:
    LoaderBundle(sp<AssetBundle> assetBundle, sp<LoaderType> defaultLoader = nullptr)
        : _asset_bundle(std::move(assetBundle)), _default_loader(std::move(defaultLoader)) {
    }

    T get(const String& name) override {
        const auto [fname, fext] = name.rcut('.');
        const sp<Asset> asset = _asset_bundle->getAsset(name);
        CHECK(asset, "Asset %s not found", name.c_str());

        return getLoader(fext)->load(asset->open());
    }

    void addLoader(String ext, sp<LoaderType> loader) {
        _loaders[std::move(ext)] = std::move(loader);
    }

    const sp<LoaderType>& getLoader(const String& ext) const {
        const auto iter = _loaders.find(ext);
        if(iter != _loaders.end())
            return iter->second;
        CHECK(_default_loader, "Cannot find loader for the extension \"%s\"", ext.c_str());
        return _default_loader;
    }

private:
    sp<AssetBundle> _asset_bundle;
    sp<LoaderType> _default_loader;
    Map<String, sp<LoaderType>> _loaders;
};

}
