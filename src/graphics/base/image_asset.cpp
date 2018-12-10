#include "graphics/base/image_asset.h"

#include "core/inf/asset.h"
#include "core/inf/loader.h"
#include "core/util/strings.h"

namespace ark {

ImageAsset::ImageAsset(const sp<Asset>& asset, const sp<BitmapLoader>& defaultLoader)
    : _asset(asset), _default_loader(defaultLoader)
{
}

bitmap ImageAsset::get(const String& name)
{
    String fname, fext;
    Strings::rcut(name, fname, fext, '.');

    const sp<Readable> readable = _asset->get(name);
    DCHECK(readable, "Asset %s not found", name.c_str());

    return getLoader(fext)->load(readable);
}

void ImageAsset::addLoader(const String& ext, const sp<BitmapLoader>& loader)
{
    _loaders[ext] = loader;
}

const sp<BitmapLoader>& ImageAsset::getLoader(const String& ext) const
{
    const auto iter = _loaders.find(ext);
    if(iter != _loaders.end())
        return iter->second;
    DCHECK(_default_loader, "Counld not find loader for image extension \"%s\"", ext.c_str());
    return _default_loader;
}

}
