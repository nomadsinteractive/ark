#include "graphics/base/image_bundle.h"

#include "core/inf/asset.h"
#include "core/inf/loader.h"
#include "core/util/strings.h"

namespace ark {

ImageBundle::ImageBundle(const sp<Asset>& asset, const sp<BitmapLoader>& defaultLoader)
    : _asset(asset), _default_loader(defaultLoader)
{
}

bitmap ImageBundle::get(const String& name)
{
    String fname, fext;
    Strings::rcut(name, fname, fext, '.');

    const sp<Readable> readable = _asset->get(name);
    DCHECK(readable, "Asset %s not found", name.c_str());

    return getLoader(fext)->load(readable);
}

void ImageBundle::addLoader(const String& ext, const sp<BitmapLoader>& loader)
{
    _loaders[ext] = loader;
}

const sp<BitmapLoader>& ImageBundle::getLoader(const String& ext) const
{
    const auto iter = _loaders.find(ext);
    if(iter != _loaders.end())
        return iter->second;
    DCHECK(_default_loader, "Counld not find loader for image extension \"%s\"", ext.c_str());
    return _default_loader;
}

}